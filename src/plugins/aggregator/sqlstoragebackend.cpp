/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sqlstoragebackend.h"
#include <stdexcept>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSqlError>
#include <QThread>
#include <QVariant>
#include <QSqlRecord>
#include <QHash>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/db/oral/pgimpl.h>
#include <util/xpc/defaulthookproxy.h>
#include <util/sll/containerconversions.h>
#include <util/sll/functor.h>
#include <util/sys/paths.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	namespace oral = Util::oral;
	namespace sph = Util::oral::sph;

	QString CommonFieldNameMorpher (QString str)
	{
		str.chop (1);

		for (int i = 1; i < str.size (); ++i)
		{
			auto ch = str [i];
			auto prev = str [i - 1];

			if (ch.isLetter () &&
					prev.isLetter () &&
					ch.isUpper () &&
					prev.isLower ())
				str.insert (i, '_');
		}

		return std::move (str).toLower ();
	}

	using PKey_t = oral::PKey<IDType_t, oral::NoAutogen>;

	struct Tags
	{
		QStringList TagsList_;

		Tags () = default;

		Tags (QStringList tags)
		: TagsList_ { std::move (tags) }
		{
		}

		operator QStringList () const
		{
			return TagsList_;
		}

		using BaseType = QString;

		// need to migrate off this in the next schema migration
		static const BaseType EmptyMarker_;

		BaseType ToBaseType () const
		{
			if (TagsList_.isEmpty ())
				return EmptyMarker_;

			static const auto itm = GetProxyHolder ()->GetTagsManager ();
			return itm->Join (TagsList_);
		}

		static Tags FromBaseType (const BaseType& var)
		{
			if (var == EmptyMarker_)
				return {};

			static const auto itm = GetProxyHolder ()->GetTagsManager ();
			return { itm->Split (var) };
		}
	};

	const QString Tags::EmptyMarker_ = "<<<null>>>";

	struct Image
	{
		QImage Image_;

		Image () = default;

		Image (QImage image)
		: Image_ { std::move (image) }
		{
		}

		operator QImage () const
		{
			return Image_;
		}

		using BaseType = QByteArray;

		BaseType ToBaseType () const
		{
			QByteArray bytes;
			if (!Image_.isNull ())
			{
				QBuffer buffer (&bytes);
				buffer.open (QIODevice::WriteOnly);
				Image_.save (&buffer, "PNG");
			}
			return bytes;
		}

		static Image FromBaseType (const BaseType& var)
		{
			QImage result;
			if (!var.isEmpty ())
				result.loadFromData (var, "PNG");
			return { result };
		}
	};

	struct ItemCategories
	{
		QStringList Categories_;

		ItemCategories () = default;

		ItemCategories (QStringList cats)
		: Categories_ { std::move (cats) }
		{
		}

		operator QStringList () const
		{
			return Categories_;
		}

		using BaseType = QString;

		BaseType ToBaseType () const
		{
			return Categories_.join ("<<<");
		}

		static ItemCategories FromBaseType (const BaseType& var)
		{
			return { var.split ("<<<", Qt::SkipEmptyParts) };
		}
	};

	struct GeoCoord
	{
		double Coord_ = 0;

		GeoCoord () = default;

		GeoCoord (double c)
		: Coord_ { c }
		{
		}

		operator double () const
		{
			return Coord_;
		}

		using BaseType = QString;

		BaseType ToBaseType () const
		{
			return QString::number (Coord_);
		}

		static GeoCoord FromBaseType (const BaseType& var)
		{
			return { var.toDouble () };
		}
	};

	template<typename OralType, typename Src>
	OralType ToOralType (const Src& src)
	{
		if constexpr (std::is_same_v<OralType, std::decay_t<Src>>)
			return src;
		else
			return { src };
	}

	template<typename T>
	decltype (auto) FromOralType (const T& src)
	{
		if constexpr (oral::IsIndirect<T> {})
			return *src;
		else
			return src;
	}
}

#define DEFINE_FIELD(_1, _2, triple) \
		BOOST_PP_TUPLE_ELEM (0, triple) BOOST_PP_TUPLE_ELEM (1, triple);

#define DEFINE_FROM_FIELD(_1, _2, triple) \
		ToOralType<BOOST_PP_TUPLE_ELEM (0, triple)> (orig.BOOST_PP_TUPLE_ELEM (2, triple)),

#define DEFINE_TO_FIELD(_1, _2, triple) \
		res.BOOST_PP_TUPLE_ELEM (2, triple) = FromOralType (BOOST_PP_TUPLE_ELEM (1, triple));

#define EXTRACT_NAME(_1, _2, triple) BOOST_PP_TUPLE_ELEM (1, triple),

#define DEFINE_STRUCT(structName, className, origStructName, fields)					\
namespace LC::Aggregator														\
{																						\
	struct SQLStorageBackend::structName												\
	{																					\
		BOOST_PP_SEQ_FOR_EACH (DEFINE_FIELD, _, fields)									\
																						\
		static QString ClassName () { return className; }								\
																						\
		static constexpr auto FieldNameMorpher = &CommonFieldNameMorpher;				\
																						\
		static structName FromOrig (const origStructName& orig)							\
		{																				\
			return { BOOST_PP_SEQ_FOR_EACH (DEFINE_FROM_FIELD, _, fields) };			\
		}																				\
																						\
		origStructName ToOrig () const													\
		{																				\
			origStructName res;															\
			BOOST_PP_SEQ_FOR_EACH (DEFINE_TO_FIELD, _, fields)							\
			return res;																	\
		}																				\
	};																					\
}																						\
																						\
BOOST_FUSION_ADAPT_STRUCT(LC::Aggregator::SQLStorageBackend::structName,		\
		BOOST_PP_SEQ_FOR_EACH (EXTRACT_NAME, _, fields))

#define SAME_NAME(type, name) (type, name, name)

DEFINE_STRUCT (FeedR, "feeds", Feed,
		(SAME_NAME (PKey_t, FeedID_))
		(SAME_NAME (oral::UniqueNotNull<QString>, URL_))
		(SAME_NAME (QDateTime, LastUpdate_))
		)
DEFINE_STRUCT (FeedSettingsR, "feeds_settings", Feed::FeedSettings,
		(SAME_NAME (oral::References<&FeedR::FeedID_>, FeedID_))
		(SAME_NAME (oral::NotNull<int>, UpdateTimeout_))
		(SAME_NAME (oral::NotNull<int>, NumItems_))
		(SAME_NAME (oral::NotNull<int>, ItemAge_))
		(SAME_NAME (oral::NotNull<bool>, AutoDownloadEnclosures_))
		)
DEFINE_STRUCT (ChannelR, "channels", Channel,
		(SAME_NAME (PKey_t, ChannelID_))
		(SAME_NAME (oral::References<&FeedR::FeedID_>, FeedID_))
		((QString, URL_, Link_))
		(SAME_NAME (QString, Title_))
		(SAME_NAME (QString, DisplayTitle_))
		(SAME_NAME (QString, Description_))
		(SAME_NAME (QDateTime, LastBuild_))
		(SAME_NAME (Tags, Tags_))
		(SAME_NAME (QString, Language_))
		(SAME_NAME (QString, Author_))
		(SAME_NAME (QString, PixmapURL_))
		(SAME_NAME (Image, Pixmap_))
		(SAME_NAME (Image, Favicon_))
		)
DEFINE_STRUCT (ItemR, "items", Item,
		(SAME_NAME (PKey_t, ItemID_))
		(SAME_NAME (oral::References<&ChannelR::ChannelID_>, ChannelID_))
		(SAME_NAME (QString, Title_))
		((QString, URL_, Link_))
		(SAME_NAME (QString, Description_))
		(SAME_NAME (QString, Author_))
		((ItemCategories, Category_, Categories_))
		(SAME_NAME (QString, Guid_))
		(SAME_NAME (QDateTime, PubDate_))
		(SAME_NAME (bool, Unread_))
		(SAME_NAME (int, NumComments_))
		((QString, CommentsUrl_, CommentsLink_))
		((QString, CommentsPageUrl_, CommentsPageLink_))
		(SAME_NAME (GeoCoord, Latitude_))
		(SAME_NAME (GeoCoord, Longitude_))
		)
DEFINE_STRUCT (EnclosureR, "enclosures", Enclosure,
		(SAME_NAME (PKey_t, EnclosureID_))
		(SAME_NAME (oral::References<&ItemR::ItemID_>, ItemID_))
		(SAME_NAME (oral::NotNull<QString>, URL_))
		(SAME_NAME (oral::NotNull<QString>, Type_))
		(SAME_NAME (oral::NotNull<qint64>, Length_))
		(SAME_NAME (QString, Lang_))
		)
DEFINE_STRUCT (MRSSEntryR, "mrss", MRSSEntry,
		((PKey_t, MrssID_, MRSSEntryID_))
		(SAME_NAME (oral::References<&ItemR::ItemID_>, ItemID_))
		(SAME_NAME (QString, URL_))
		(SAME_NAME (qint64, Size_))
		(SAME_NAME (QString, Type_))
		(SAME_NAME (QString, Medium_))
		(SAME_NAME (bool, IsDefault_))
		(SAME_NAME (QString, Expression_))
		(SAME_NAME (int, Bitrate_))
		(SAME_NAME (double, Framerate_))
		((double, Samplingrate_, SamplingRate_))
		(SAME_NAME (int, Channels_))
		(SAME_NAME (int, Duration_))
		(SAME_NAME (int, Width_))
		(SAME_NAME (int, Height_))
		(SAME_NAME (QString, Lang_))
		((int, Mediagroup_, Group_))
		(SAME_NAME (QString, Rating_))
		(SAME_NAME (QString, RatingScheme_))
		(SAME_NAME (QString, Title_))
		(SAME_NAME (QString, Description_))
		(SAME_NAME (QString, Keywords_))
		(SAME_NAME (QString, CopyrightURL_))
		(SAME_NAME (QString, CopyrightText_))
		((int, StarRatingAverage_, RatingAverage_))
		((int, StarRatingCount_, RatingCount_))
		((int, StarRatingMin_, RatingMin_))
		((int, StarRatingMax_, RatingMax_))
		((int, StatViews_, Views_))
		((int, StatFavs_, Favs_))
		(SAME_NAME (QString, Tags_))
		)
DEFINE_STRUCT (MRSSThumbnailR, "mrss_thumbnails", MRSSThumbnail,
		((PKey_t, MrssThumbID_, MRSSThumbnailID_))
		((oral::References<&MRSSEntryR::MrssID_>, MrssID_, MRSSEntryID_))
		(SAME_NAME (QString, URL_))
		(SAME_NAME (int, Width_))
		(SAME_NAME (int, Height_))
		(SAME_NAME (QString, Time_))
		)
DEFINE_STRUCT (MRSSCreditR, "mrss_credits", MRSSCredit,
		((PKey_t, MrssCreditsID_, MRSSCreditID_))
		((oral::References<&MRSSEntryR::MrssID_>, MrssID_, MRSSEntryID_))
		(SAME_NAME (QString, Role_))
		(SAME_NAME (QString, Who_))
		)
DEFINE_STRUCT (MRSSCommentR, "mrss_comments", MRSSComment,
		((PKey_t, MrssCommentID_, MRSSCommentID_))
		((oral::References<&MRSSEntryR::MrssID_>, MrssID_, MRSSEntryID_))
		(SAME_NAME (QString, Type_))
		(SAME_NAME (QString, Comment_))
		)
DEFINE_STRUCT (MRSSPeerLinkR, "mrss_peerlinks", MRSSPeerLink,
		((PKey_t, MrssPeerlinkID_, MRSSPeerLinkID_))
		((oral::References<&MRSSEntryR::MrssID_>, MrssID_, MRSSEntryID_))
		(SAME_NAME (QString, Type_))
		(SAME_NAME (QString, Link_))
		)
DEFINE_STRUCT (MRSSSceneR, "mrss_scenes", MRSSScene,
		((PKey_t, MrssSceneID_, MRSSSceneID_))
		((oral::References<&MRSSEntryR::MrssID_>, MrssID_, MRSSEntryID_))
		(SAME_NAME (QString, Title_))
		(SAME_NAME (QString, Description_))
		(SAME_NAME (QString, StartTime_))
		(SAME_NAME (QString, EndTime_))
		)

namespace LC::Aggregator
{
	struct SQLStorageBackend::Item2TagsR
	{
		oral::References<&ItemR::ItemID_> ItemID_;
		oral::NotNull<QString> Tag_;

		static QString ClassName ()
		{
			return "items2tags";
		}

		static constexpr auto FieldNameMorpher = &CommonFieldNameMorpher;
	};

	struct SQLStorageBackend::Feed2TagsR
	{
		oral::Unique<oral::References<&FeedR::FeedID_>> FeedID_;
		oral::NotNull<Tags> Tags_;

		static QString ClassName ()
		{
			return "feeds2tags";
		}

		static constexpr auto FieldNameMorpher = &CommonFieldNameMorpher;
	};
}

BOOST_FUSION_ADAPT_STRUCT (LC::Aggregator::SQLStorageBackend::Item2TagsR,
		ItemID_,
		Tag_)
BOOST_FUSION_ADAPT_STRUCT (LC::Aggregator::SQLStorageBackend::Feed2TagsR,
		FeedID_,
		Tags_)

namespace LC::Aggregator
{
	using Util::operator*;

	SQLStorageBackend::SQLStorageBackend (StorageBackend::Type t, const QString& id)
	: Type_ (t)
	{
		QString strType;
		switch (Type_)
		{
		case SBSQLite:
			strType = "QSQLITE";
			break;
		case SBPostgres:
			strType = "QPSQL";
			break;
		case SBMysql:
			break;
		}

		DB_ = QSqlDatabase::addDatabase (strType, Util::GenConnectionName ("org.LeechCraft.Aggregator" + id));

		switch (Type_)
		{
		case SBSQLite:
		{
			auto dir = Util::GetUserDir (Util::UserDir::LC, "aggregator");
			DB_.setDatabaseName (dir.filePath ("aggregator.db"));
			break;
		}
		case SBPostgres:
		{
			DB_.setDatabaseName (XmlSettingsManager::Instance ()->property ("PostgresDBName").toString ());
			DB_.setHostName (XmlSettingsManager::Instance ()->property ("PostgresHostname").toString ());
			DB_.setPort (XmlSettingsManager::Instance ()->property ("PostgresPort").toInt ());
			DB_.setUserName (XmlSettingsManager::Instance ()->property ("PostgresUsername").toString ());
			DB_.setPassword (XmlSettingsManager::Instance ()->property ("PostgresPassword").toString ());
			break;
		}
		case SBMysql:
			break;
		}

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO;
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
						.arg (DB_.lastError ().text ())));
		}

		auto adaptedPtrs = std::tie (Feeds_, FeedsSettings_, Channels_, Items_, Enclosures_,
				MRSSEntries_, MRSSThumbnails_, MRSSCredits_, MRSSComments_, MRSSPeerLinks_, MRSSScenes_,
				Items2Tags_, Feeds2Tags_);
		Type_ == SBSQLite ?
				oral::AdaptPtrs<oral::SQLiteImplFactory> (DB_, adaptedPtrs) :
				oral::AdaptPtrs<oral::PostgreSQLImplFactory> (DB_, adaptedPtrs);

		DBRemover_ = Util::MakeScopeGuard ([conn = DB_.connectionName ()] { QSqlDatabase::removeDatabase (conn); });
	}

	void SQLStorageBackend::Prepare ()
	{
		if (Type_ == SBSQLite)
		{
			Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");
			Util::RunTextQuery (DB_, "PRAGMA foreign_keys = ON;");
		}
	}

	ids_t SQLStorageBackend::GetFeedsIDs () const
	{
		return Feeds_->Select.Build ()
				.Select (sph::fields<&FeedR::FeedID_>)
				.Order (oral::OrderBy<sph::asc<&FeedR::FeedID_>>)
				();
	}

	QList<ITagsManager::tag_id> SQLStorageBackend::GetItemTags (IDType_t id)
	{
		return Items2Tags_->Select (sph::fields<&Item2TagsR::Tag_>, sph::f<&Item2TagsR::ItemID_> == id);
	}

	void SQLStorageBackend::SetItemTags (IDType_t id, const QList<ITagsManager::tag_id>& tags)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		Items2Tags_->DeleteBy (sph::f<&Item2TagsR::ItemID_> == id);

		for (const auto& tag : tags)
			Items2Tags_->Insert ({ id, tag });

		lock.Good ();

		if (const auto& item = GetItem (id))
			emit itemDataUpdated (*item);
	}

	QList<IDType_t> SQLStorageBackend::GetItemsForTag (const ITagsManager::tag_id& tag)
	{
		return Items2Tags_->Select (sph::fields<&Item2TagsR::ItemID_>, sph::f<&Item2TagsR::Tag_> == tag);
	}

	IDType_t SQLStorageBackend::GetHighestID (const PoolType& type) const
	{
		static const QHash<PoolType, QPair<QByteArray, QByteArray>> tables
		{
			{ PTFeed, { "feed_id", "feeds" } },
			{ PTChannel, { "channel_id", "channels" } },
			{ PTItem, { "item_id", "items" } },
			{ PTEnclosure, { "enclosure_id", "enclosures" } },
			{ PTMRSSEntry, { "mrss_id", "mrss" } },
			{ PTMRSSThumbnail, { "mrss_thumb_id", "mrss_thumbnails" } },
			{ PTMRSSCredit, { "mrss_credits_id", "mrss_credits" } },
			{ PTMRSSComment, { "mrss_comment_id", "mrss_comments" } },
			{ PTMRSSPeerLink, { "mrss_peerlink_id", "mrss_peerlinks" } },
			{ PTMRSSScene, { "mrss_scene_id", "mrss_scenes" } },
		};

		const auto& [field, table] = tables.value (type);
		if (field.isEmpty ())
		{
			qWarning () << "GetHighestID(): unknown pool type" << type;
			return 0;
		}

		return GetHighestID (field, table);
	}

	IDType_t SQLStorageBackend::GetHighestID (const QByteArray& idName, const QByteArray& tableName) const
	{
		QSqlQuery findHighestID (DB_);
		//due to some strange troubles with QSqlQuery::bindValue ()
		//we'll bind values by ourselves. It should be safe as this is our
		//internal function.
		if (!findHighestID.exec (QString ("SELECT MAX (%1) FROM %2")
						.arg (idName, tableName)))
		{
			Util::DBLock::DumpError (findHighestID);
			return 0;
		}

		if (findHighestID.first ())
			return findHighestID.value (0).value<IDType_t> ();
		else
			return 0;
	}

	Feed SQLStorageBackend::GetFeed (IDType_t feedId) const
	{
		const auto maybeFeed = Feeds_->SelectOne (sph::f<&FeedR::FeedID_> == feedId);
		if (!maybeFeed)
		{
			qWarning () << Q_FUNC_INFO
					<< "no feed found with"
					<< feedId;
			throw FeedNotFoundError {};
		}

		return maybeFeed->ToOrig ();
	}

	std::optional<IDType_t> SQLStorageBackend::FindFeed (const QString& url) const
	{
		return Feeds_->SelectOne (sph::fields<&FeedR::FeedID_>,
		        sph::f<&FeedR::URL_> == url);
	}

	std::optional<Feed::FeedSettings> SQLStorageBackend::GetFeedSettings (IDType_t feedId) const
	{
		return FeedsSettings_->SelectOne (sph::f<&FeedSettingsR::FeedID_> == feedId) * &FeedSettingsR::ToOrig;
	}

	void SQLStorageBackend::SetFeedSettings (const Feed::FeedSettings& settings)
	{
		FeedsSettings_->Insert (FeedSettingsR::FromOrig (settings),
				oral::InsertAction::Replace::Fields<&FeedSettingsR::FeedID_>);
	}

	std::optional<QStringList> SQLStorageBackend::GetFeedTags (IDType_t feedId) const
	{
		return Feeds2Tags_->SelectOne (sph::f<&Feed2TagsR::FeedID_> == feedId) *
				[] (const auto& feed2tags) { return static_cast<QStringList> (*feed2tags.Tags_); };
	}

	void SQLStorageBackend::SetFeedTags (IDType_t feedId, const QStringList& tags)
	{
		Feeds2Tags_->Insert (Feed2TagsR { feedId, tags },
				oral::InsertAction::Replace::Fields<&Feed2TagsR::FeedID_>);
	}

	void SQLStorageBackend::SetFeedURL (IDType_t feedId, const QString& url)
	{
		Feeds_->Update (sph::f<&FeedR::URL_> = url, sph::f<&FeedR::FeedID_> == feedId);
	}

	channels_shorts_t SQLStorageBackend::GetChannels (IDType_t feedId) const
	{
		constexpr auto shortFields = sph::fields<
					&ChannelR::ChannelID_,
					&ChannelR::FeedID_,
					&ChannelR::Author_,
					&ChannelR::Title_,
					&ChannelR::DisplayTitle_,
					&ChannelR::URL_,
					&ChannelR::Tags_,
					&ChannelR::LastBuild_,
					&ChannelR::Favicon_
				>;
		auto shortsTuples = Channels_->Select.Build ()
				.Where (sph::f<&ChannelR::FeedID_> == feedId)
				.Select (shortFields)
				.Order (oral::OrderBy<sph::asc<&ChannelR::Title_>>)
				();

		channels_shorts_t shorts;

		for (auto& shortTuple : shortsTuples)
		{
			const auto cid = std::get<0> (shortTuple);

			auto cs = std::make_from_tuple<ChannelShort> (std::move (shortTuple));
			cs.Unread_ = GetUnreadItemsCount (cid);
			shorts.push_back (std::move (cs));
		}

		return shorts;
	}

	Channel SQLStorageBackend::GetChannel (IDType_t channelId) const
	{
		const auto maybeChannel = Channels_->SelectOne (sph::f<&ChannelR::ChannelID_> == channelId);
		if (!maybeChannel)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find"
					<< channelId;
			throw ChannelNotFoundError {};
		}
		return maybeChannel->ToOrig ();
	}

	std::optional<IDType_t> SQLStorageBackend::FindChannel (const QString& title,
			const QString& link, IDType_t feedId) const
	{
		return Channels_->SelectOne (sph::fields<&ChannelR::ChannelID_>,
				sph::f<&ChannelR::Title_> == title &&
				sph::f<&ChannelR::URL_> == link &&
				sph::f<&ChannelR::FeedID_> == feedId);
	}

	std::optional<IDType_t> SQLStorageBackend::FindItem (const QString& title,
			const QString& link, IDType_t channelId) const
	{
		return Items_->SelectOne (sph::fields<&ItemR::ItemID_>,
				sph::f<&ItemR::ChannelID_> == channelId &&
				sph::f<&ItemR::Title_> == title &&
				sph::f<&ItemR::URL_> == link);
	}

	std::optional<IDType_t> SQLStorageBackend::FindItemByLink (const QString& link,
			IDType_t channelId) const
	{
		if (link.isEmpty ())
			return {};

		return Items_->SelectOne (sph::fields<&ItemR::ItemID_>,
				sph::f<&ItemR::ChannelID_> == channelId &&
				sph::f<&ItemR::URL_> == link);
	}

	std::optional<IDType_t> SQLStorageBackend::FindItemByTitle (const QString& title,
			IDType_t channelId) const
	{
		return Items_->SelectOne (sph::fields<&ItemR::ItemID_>,
				sph::f<&ItemR::ChannelID_> == channelId &&
				sph::f<&ItemR::Title_> == title);
	}

	void SQLStorageBackend::TrimChannel (IDType_t channelId,
			int days, int number)
	{
		const auto& cutoff = QDateTime::currentDateTime ().addDays (-days);

		auto removeByDate = Items_->Select (sph::fields<&ItemR::ItemID_>,
				sph::f<&ItemR::ChannelID_> == channelId &&
				sph::f<&ItemR::Unread_> == false &&
				sph::f<&ItemR::PubDate_> < cutoff);
		auto removeByCount = Items_->Select.Build ()
				.Select (sph::fields<&ItemR::ItemID_>)
				.Where (sph::f<&ItemR::ChannelID_> == channelId && sph::f<&ItemR::Unread_> == false)
				.Order (oral::OrderBy<sph::desc<&ItemR::PubDate_>>)
				.Offset (number)
				();

		const auto& removedIds = Util::AsSet (removeByDate) + Util::AsSet (removeByCount);

		emit itemsRemoved (removedIds);

		Util::DBLock lock (DB_);
		lock.Init ();
		for (auto id : removedIds)
			Items_->DeleteBy (sph::f<&ItemR::ItemID_> == id);
		lock.Good ();
	}

	std::optional<QImage> SQLStorageBackend::GetChannelPixmap (IDType_t channelId) const
	{
		return Channels_->SelectOne (sph::fields<&ChannelR::Pixmap_>, sph::f<&ChannelR::ChannelID_> == channelId);
	}

	void SQLStorageBackend::SetChannelPixmap (IDType_t id, const std::optional<QImage>& img)
	{
		// TODO no need for value_or when oral will support setting NULL
		Channels_->Update (sph::f<&ChannelR::Pixmap_> = img.value_or (QImage {}), sph::f<&ChannelR::ChannelID_> == id);
		emit channelDataUpdated (GetChannel (id));
	}

	void SQLStorageBackend::SetChannelFavicon (IDType_t id, const std::optional<QImage>& img)
	{
		Channels_->Update (sph::f<&ChannelR::Favicon_> = img.value_or (QImage {}), sph::f<&ChannelR::ChannelID_> == id);
		emit channelDataUpdated (GetChannel (id));
	}

	void SQLStorageBackend::SetChannelTags (IDType_t id, const QStringList& tagIds)
	{
		Channels_->Update (sph::f<&ChannelR::Tags_> = tagIds, sph::f<&ChannelR::ChannelID_> == id);
		emit channelDataUpdated (GetChannel (id));
	}

	void SQLStorageBackend::SetChannelDisplayTitle (IDType_t id, const QString& displayTitle)
	{
		Channels_->Update (sph::f<&ChannelR::DisplayTitle_> = displayTitle, sph::f<&ChannelR::ChannelID_> == id);
		emit channelDataUpdated (GetChannel (id));
	}

	void SQLStorageBackend::SetChannelTitle (IDType_t id, const QString& title)
	{
		Channels_->Update (sph::f<&ChannelR::Title_> = title, sph::f<&ChannelR::ChannelID_> == id);
		emit channelDataUpdated (GetChannel (id));
	}

	void SQLStorageBackend::SetChannelLink (IDType_t id, const QString& link)
	{
		Channels_->Update (sph::f<&ChannelR::URL_> = link, sph::f<&ChannelR::ChannelID_> == id);
		emit channelDataUpdated (GetChannel (id));
	}

	items_shorts_t SQLStorageBackend::GetItems (IDType_t channelId) const
	{
		constexpr auto shortFields = sph::fields<
					&ItemR::ItemID_,
					&ItemR::ChannelID_,
					&ItemR::Title_,
					&ItemR::URL_,
					&ItemR::Category_,
					&ItemR::PubDate_,
					&ItemR::Unread_
				>;
		auto rawTuples = Items_->Select (shortFields, sph::f<&ItemR::ChannelID_> == channelId);
		return Util::MapAs<QVector> (std::move (rawTuples),
				[]<typename Tup> (Tup&& tup) { return std::make_from_tuple<ItemShort> (std::forward<Tup> (tup)); });
	}

	int SQLStorageBackend::GetUnreadItemsCount (IDType_t channelId) const
	{
		return Items_->Select (sph::count<>,
				sph::f<&ItemR::ChannelID_> == channelId && sph::f<&ItemR::Unread_> == true);
	}

	int SQLStorageBackend::GetTotalItemsCount (IDType_t channelId) const
	{
		return Items_->Select (sph::count<>, sph::f<&ItemR::ChannelID_> == channelId);
	}

	std::optional<Item> SQLStorageBackend::GetItem (IDType_t itemId) const
	{
		const auto maybeItem = Items_->SelectOne (sph::f<&ItemR::ItemID_> == itemId);
		if (!maybeItem)
			return {};

		auto item = maybeItem->ToOrig ();
		GetEnclosures (itemId, item.Enclosures_);
		GetMRSSEntries (itemId, item.MRSSEntries_);
		emit hookItemLoad (std::make_shared<Util::DefaultHookProxy> (), &item);
		return item;
	}

	void SQLStorageBackend::AddFeed (const Feed& feed)
	{
		Feeds_->Insert (FeedR::FromOrig (feed));

		try
		{
			for (const auto& chan : feed.Channels_)
				AddChannel (*chan);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
		}
	}

	void SQLStorageBackend::UpdateItem (const Item& item)
	{
		Items_->Update (ItemR::FromOrig (item));

		Enclosures_->DeleteBy (sph::f<&ItemR::ItemID_> == item.ItemID_);
		WriteEnclosures (item.Enclosures_);
		WriteMRSSEntries (item.MRSSEntries_);

		emit itemDataUpdated (item);
	}

	void SQLStorageBackend::SetItemUnread (IDType_t itemId, bool unread)
	{
		Items_->Update (sph::f<&ItemR::Unread_> = unread, sph::f<&ItemR::ItemID_> == itemId);

		if (const auto& fullItem = GetItem (itemId))
		{
			const auto channelId = fullItem->ChannelID_;
			emit itemReadStatusUpdated (channelId, itemId, unread);
			emit channelUnreadCountUpdated (channelId, GetUnreadItemsCount (channelId));
		}
	}

	void SQLStorageBackend::AddChannel (const Channel& channel)
	{
		Channels_->Insert (ChannelR::FromOrig (channel));
		for (const auto& item : channel.Items_)
			AddItem (*item);
		emit channelAdded (channel);
	}

	void SQLStorageBackend::AddItem (const Item& item)
	{
		Items_->Insert (ItemR::FromOrig (item));

		WriteEnclosures (item.Enclosures_);
		WriteMRSSEntries (item.MRSSEntries_);

		emit hookItemAdded (std::make_shared<Util::DefaultHookProxy> (), item);

		emit itemDataUpdated (item);
		emit channelUnreadCountUpdated (item.ChannelID_, GetUnreadItemsCount (item.ChannelID_));
	}

	void SQLStorageBackend::RemoveItems (const QSet<IDType_t>& items)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		QList<IDType_t> modifiedChannels;
		for (const auto itemId : items)
		{
			const auto& cid = Items_->SelectOne (sph::fields<&ItemR::ChannelID_>, sph::f<&ItemR::ItemID_> == itemId);
			if (!cid)
				continue;

			if (!modifiedChannels.contains (*cid))
				modifiedChannels << *cid;

			Items_->DeleteBy (sph::f<&ItemR::ItemID_> == itemId);
		}

		lock.Good ();

		emit itemsRemoved (items);

		for (const auto& cid : modifiedChannels)
			emit channelUnreadCountUpdated (cid, GetUnreadItemsCount (cid));
	}

	void SQLStorageBackend::RemoveChannel (IDType_t channelId)
	{
		Util::DBLock lock (DB_);
		lock.Init ();
		Channels_->DeleteBy (sph::f<&ChannelR::ChannelID_> == channelId);
		lock.Good ();
		emit channelRemoved (channelId);
	}

	void SQLStorageBackend::RemoveFeed (IDType_t feedId)
	{
		Util::DBLock lock (DB_);
		lock.Init ();
		Feeds_->DeleteBy (sph::f<&FeedR::FeedID_> == feedId);
		lock.Good ();
		emit feedRemoved (feedId);
	}

	void SQLStorageBackend::ToggleChannelUnread (IDType_t channelId, bool state)
	{
		const auto oldItems = Items_->Select (sph::fields<&ItemR::ItemID_, &ItemR::Unread_>,
				sph::f<&ItemR::ChannelID_> == channelId);

		Items_->Update (sph::f<&ItemR::Unread_> = state,
				sph::f<&ItemR::ChannelID_> == channelId);

		emit channelUnreadCountUpdated (channelId, state ? oldItems.size () : 0);

		for (const auto& [itemId, oldState] : oldItems)
			if (oldState != state)
				emit itemReadStatusUpdated (channelId, itemId, state);
	}

	bool SQLStorageBackend::UpdateFeedsStorage (int from)
	{
		Util::DBLock lock { DB_ };
		lock.Init ();
		try
		{
			if (from <= 1)
			{
				qDebug () << Q_FUNC_INFO << "migrating tags";

				const auto& feedsTags = Feeds2Tags_->Select ();

				Util::RunTextQuery (DB_, "DROP TABLE " + Feed2TagsR::ClassName ());

				Feeds2Tags_ = Type_ == SBSQLite ?
						oral::AdaptPtr<Feed2TagsR, oral::SQLiteImplFactory> (DB_) :
						oral::AdaptPtr<Feed2TagsR, oral::PostgreSQLImplFactory> (DB_);

				for (const auto& [feedId, tags] : feedsTags)
					SetFeedTags (*feedId, tags->TagsList_);
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return false;
		}
		lock.Good ();
		return true;
	}

	bool SQLStorageBackend::UpdateChannelsStorage (int)
	{
		qCritical () << Q_FUNC_INFO
				<< "support for old channel storage tables dropped";
		return false;
	}

	bool SQLStorageBackend::UpdateItemsStorage (int)
	{
		qCritical () << Q_FUNC_INFO
				<< "support for old items storage tables dropped";
		return false;
	}

	void SQLStorageBackend::WriteEnclosures (const QList<Enclosure>& enclosures)
	{
		for (const auto& enclosure : enclosures)
			Enclosures_->Insert (EnclosureR::FromOrig (enclosure), oral::InsertAction::Replace::PKey<EnclosureR>);
	}

	void SQLStorageBackend::GetEnclosures (IDType_t itemId, QList<Enclosure>& enclosures) const
	{
		enclosures = Util::Map (Enclosures_->Select (sph::f<&EnclosureR::ItemID_> == itemId), &EnclosureR::ToOrig);
	}

	namespace
	{
		template<typename RecType, typename OrigType>
		void InsertList (const oral::ObjectInfo_ptr<RecType>& records, const QList<OrigType>& origs)
		{
			for (const auto& orig : origs)
				records->Insert (RecType::FromOrig (orig), oral::InsertAction::Replace::PKey<RecType>);
		}
	}

	void SQLStorageBackend::WriteMRSSEntries (const QList<MRSSEntry>& entries)
	{
		for (const auto& e : entries)
		{
			MRSSEntries_->Insert (MRSSEntryR::FromOrig (e), oral::InsertAction::Replace::PKey<MRSSEntryR>);
			InsertList (MRSSThumbnails_, e.Thumbnails_);
			InsertList (MRSSCredits_, e.Credits_);
			InsertList (MRSSComments_, e.Comments_);
			InsertList (MRSSPeerLinks_, e.PeerLinks_);
			InsertList (MRSSScenes_, e.Scenes_);
		}
	}

	namespace
	{
		template<auto PKey, typename RecType>
		auto SelectMapping (const oral::ObjectInfo_ptr<RecType>& records, IDType_t pkeyValue)
		{
			return Util::Map (records->Select (sph::f<PKey> == pkeyValue), &RecType::ToOrig);
		}
	}

	void SQLStorageBackend::GetMRSSEntries (IDType_t itemId, QList<MRSSEntry>& entries) const
	{
		entries = SelectMapping<&MRSSEntryR::ItemID_> (MRSSEntries_, itemId);
		for (auto& entry : entries)
		{
			auto mrssId = entry.MRSSEntryID_;
			entry.Thumbnails_ = SelectMapping<&MRSSThumbnailR::MrssID_> (MRSSThumbnails_, mrssId);
			entry.Credits_ = SelectMapping<&MRSSCreditR::MrssID_> (MRSSCredits_, mrssId);
			entry.Comments_ = SelectMapping<&MRSSCommentR::MrssID_> (MRSSComments_, mrssId);
			entry.PeerLinks_ = SelectMapping<&MRSSPeerLinkR::MrssID_> (MRSSPeerLinks_, mrssId);
			entry.Scenes_ = SelectMapping<&MRSSSceneR::MrssID_> (MRSSScenes_, mrssId);
		}
	}
}
