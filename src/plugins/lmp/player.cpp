/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "player.h"
#include <algorithm>
#include <random>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QtConcurrentRun>
#include <QFutureSynchronizer>
#include <QTimer>
#include <QApplication>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/slotclosure.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/irestorableradiostationprovider.h>
#include "core.h"
#include "mediainfo.h"
#include "localfileresolver.h"
#include "util.h"
#include "localcollection.h"
#include "playlistmanager.h"
#include "staticplaylistmanager.h"
#include "xmlsettingsmanager.h"
#include "playlistmodel.h"
#include "playlistparsers/playlistfactory.h"
#include "engine/sourceobject.h"
#include "engine/audiosource.h"
#include "engine/output.h"
#include "engine/path.h"
#include "localcollectionmodel.h"
#include "playerrulesmanager.h"
#include "sourceerrorhandler.h"

namespace LC
{
namespace LMP
{
	Player::Sorter::Sorter ()
	{
		Criteria_ << SortingCriteria::Artist
				<< SortingCriteria::Year
				<< SortingCriteria::Album
				<< SortingCriteria::TrackNumber;
	}

	bool Player::Sorter::operator() (const MediaInfo& left, const MediaInfo& right) const
	{
		for (auto crit : Criteria_)
		{
			switch (crit)
			{
			case SortingCriteria::Artist:
				if (left.Artist_ != right.Artist_)
					return left.Artist_ < right.Artist_;
				break;
			case SortingCriteria::Year:
				if (left.Year_ != right.Year_)
					return left.Year_ < right.Year_;
				break;
			case SortingCriteria::Album:
				if (left.Album_ != right.Album_)
					return left.Album_ < right.Album_;
				break;
			case SortingCriteria::TrackNumber:
				if (left.TrackNumber_ != right.TrackNumber_)
					return left.TrackNumber_ < right.TrackNumber_;
				break;
			case SortingCriteria::TrackTitle:
				if (left.Title_ != right.Title_)
					return left.Title_ < right.Title_;
				break;
			case SortingCriteria::DirectoryPath:
			{
				const auto& leftPath = QFileInfo (left.LocalPath_).dir ().absolutePath ();
				const auto& rightPath = QFileInfo (right.LocalPath_).dir ().absolutePath ();
				if (leftPath != rightPath)
					return QString::localeAwareCompare (leftPath, rightPath) < 0;
				break;
			}
			case SortingCriteria::FileName:
			{
				const auto& leftPath = QFileInfo (left.LocalPath_).fileName ();
				const auto& rightPath = QFileInfo (right.LocalPath_).fileName ();
				if (leftPath != rightPath)
					return QString::localeAwareCompare (leftPath, rightPath) < 0;
				break;
			}
			}
		}

		return left.LocalPath_ < right.LocalPath_;
	}

	using ResolveResult_t = QList<QPair<AudioSource, MediaInfo>>;

	struct Player::ResolveJobResult
	{
		ResolveResult_t Resolved_;
		bool ShouldClear_;
	};

	Player::Player (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, PlaylistModel_ (new PlaylistModel (this))
	, Source_ (new SourceObject (Category::Music, this))
	, Output_ (new Output (this))
	, Path_ (new Path (Source_, Output_))
	, PRG_ { static_cast<std::mt19937::result_type> (QDateTime::currentSecsSinceEpoch ()) }
	, RulesManager_ (new PlayerRulesManager (PlaylistModel_, this))
	{
		qRegisterMetaType<QList<AudioSource>> ("QList<AudioSource>");
		qRegisterMetaType<StringPair_t> ("StringPair_t");
		qRegisterMetaType<std::shared_ptr<std::atomic_bool>> ("std::shared_ptr<std::atomic_bool>");

		connect (Source_,
				SIGNAL (currentSourceChanged (AudioSource)),
				this,
				SLOT (handleCurrentSourceChanged (AudioSource)));
		connect (Source_,
				SIGNAL (aboutToFinish (std::shared_ptr<std::atomic_bool>)),
				this,
				SLOT (handleUpdateSourceQueue (std::shared_ptr<std::atomic_bool>)));

		XmlSettingsManager::Instance ().RegisterObject ("SingleTrackDisplayMask",
				this, "refillPlaylist");

		const auto& criteriaVar = XmlSettingsManager::Instance ().property ("SortingCriteria");
		if (!criteriaVar.isNull ())
			Sorter_.Criteria_ = LoadCriteria (criteriaVar);

		connect (Source_,
				SIGNAL (stateChanged (SourceState, SourceState)),
				this,
				SLOT (handleStateChanged (SourceState, SourceState)));

		connect (Source_,
				SIGNAL (metaDataChanged ()),
				this,
				SLOT (handleMetadata ()));
		connect (Source_,
				SIGNAL (bufferStatus (int)),
				this,
				SIGNAL (bufferStatusChanged (int)));

		const auto seh = new SourceErrorHandler { Source_, Proxy_->GetEntityManager () };
		connect (seh,
				SIGNAL (nextTrack ()),
				this,
				SLOT (nextTrack ()));

		PlaylistModel_->setHorizontalHeaderLabels ({ tr ("Playlist") });
	}

	void Player::InitWithOtherPlugins ()
	{
		RulesManager_->InitializePlugins ();

		auto collection = Core::Instance ().GetLocalCollection ();
		if (collection->IsReady ())
			restorePlaylist ();
		else
			connect (collection,
					SIGNAL (collectionReady ()),
					this,
					SLOT (restorePlaylist ()));
	}

	QAbstractItemModel* Player::GetPlaylistModel () const
	{
		return PlaylistModel_;
	}

	SourceObject* Player::GetSourceObject () const
	{
		return Source_;
	}

	Output* Player::GetAudioOutput () const
	{
		return Output_;
	}

	Path* Player::GetPath () const
	{
		return Path_;
	}

	Player::PlayMode Player::GetPlayMode () const
	{
		return PlayMode_;
	}

	void Player::SetPlayMode (Player::PlayMode playMode)
	{
		if (PlayMode_ == playMode)
			return;

		PlayMode_ = playMode;
		emit playModeChanged (PlayMode_);
	}

	SourceState Player::GetState () const
	{
		return Source_->GetState ();
	}

	QList<SortingCriteria> Player::GetSortingCriteria () const
	{
		return Sorter_.Criteria_;
	}

	void Player::SetSortingCriteria (const QList<SortingCriteria>& criteria)
	{
		Sorter_.Criteria_ = criteria;

		AddToPlaylistModel ({}, true, false);

		XmlSettingsManager::Instance ().setProperty ("SortingCriteria", SaveCriteria (criteria));
	}

	namespace
	{
		Playlist FileToSource (const AudioSource& source)
		{
			if (!source.IsLocalFile ())
				return Playlist { { source } };

			const auto& file = source.GetLocalPath ();

			if (auto parser = MakePlaylistParser (file))
			{
				const auto& sources = parser (file);
				if (!sources.IsEmpty ())
					return sources;
			}

			return Playlist { { file } };
		}
	}

	void Player::PrepareURLInfo (const QUrl& url, const MediaInfo& info)
	{
		if (!info.IsUseless ())
			Url2Info_ [url] = info;
	}

	void Player::Enqueue (const QStringList& paths, EnqueueFlags flags)
	{
		UnsetRadio ();

		QList<AudioSource> parsedSources;
		for (const auto& path : paths)
			parsedSources << AudioSource (path);
		Enqueue (parsedSources, flags);
	}

	void Player::Enqueue (const QList<AudioSource>& sources, EnqueueFlags flags)
	{
		UnsetRadio ();

		if (CurrentQueue_.isEmpty ())
			emit shouldClearFiltering ();

		Playlist parsedSources;
		for (const auto& path : sources)
			parsedSources += FileToSource (path);

		if (!(flags & EnqueueReplace))
			for (auto i = parsedSources.begin (); i != parsedSources.end (); )
			{
				if (Items_.contains (i->Source_))
					i = parsedSources.erase (i);
				else
					++i;
			}

		const auto curSrcPos = std::find_if (parsedSources.begin (), parsedSources.end (),
				[] (const PlaylistItem& item) { return item.Additional_ ["Current"].toBool (); });
		if (curSrcPos != parsedSources.end ())
			switch (Source_->GetState ())
			{
			case SourceState::Error:
			case SourceState::Stopped:
				Source_->SetCurrentSource (curSrcPos->Source_);
				break;
			default:
				AddToOneShotQueue (curSrcPos->Source_);
				break;
			}

		AddToPlaylistModel (parsedSources.ToSources (), flags & EnqueueSort, flags & EnqueueReplace);
	}

	QList<AudioSource> Player::GetQueue () const
	{
		return CurrentQueue_;
	}

	QList<AudioSource> Player::GetIndexSources (const QModelIndex& index) const
	{
		QList<AudioSource> sources;
		if (index.data (Role::IsAlbum).toBool ())
			for (int i = 0; i < PlaylistModel_->rowCount (index); ++i)
				sources << PlaylistModel_->index (i, 0, index).data (Role::Source).value<AudioSource> ();
		else
			sources << index.data (Role::Source).value<AudioSource> ();
		return sources;
	}

	QModelIndex Player::GetSourceIndex (const AudioSource& source) const
	{
		const auto item = Items_ [source];
		return item ? item->index () : QModelIndex ();
	}

	namespace
	{
		void IncAlbumLength (QStandardItem *albumItem, int length)
		{
			const int prevLength = albumItem->data (Player::Role::AlbumLength).toInt ();
			albumItem->setData (length + prevLength, Player::Role::AlbumLength);
		}
	}

	void Player::Dequeue (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		Dequeue (GetIndexSources (index));

		if (CurrentStation_)
			UnsetRadio ();
	}

	void Player::Dequeue (const QList<AudioSource>& sources)
	{
		if (CurrentStation_)
			UnsetRadio ();

		for (const auto& source : sources)
		{
			Url2Info_.remove (source.ToUrl ());

			if (!CurrentQueue_.removeAll (source))
				continue;

			RemoveFromOneShotQueue (source);

			auto item = Items_.take (source);
			auto parent = item->parent ();
			if (parent)
			{
				if (parent->rowCount () == 1)
				{
					for (const auto& key : AlbumRoots_.keys ())
					{
						auto& items = AlbumRoots_ [key];
						if (!items.contains (parent))
							continue;

						items.removeAll (parent);
						if (items.isEmpty ())
							AlbumRoots_.remove (key);
					}
					PlaylistModel_->removeRow (parent->row ());
				}
				else
				{
					const auto& info = item->data (Role::Info).value<MediaInfo> ();
					if (!info.LocalPath_.isEmpty ())
						IncAlbumLength (parent, -info.Length_);
					parent->removeRow (item->row ());
				}
			}
			else
				PlaylistModel_->removeRow (item->row ());
		}

		SaveOnLoadPlaylist ();
	}

	AudioSource Player::GetCurrentStopSource () const
	{
		return CurrentStopSource_;
	}

	void Player::SetStopAfter (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		AudioSource stopSource;
		if (index.data (Role::IsAlbum).toBool ())
			stopSource = PlaylistModel_->index (0, 0, index).data (Role::Source).value<AudioSource> ();
		else
			stopSource = index.data (Role::Source).value<AudioSource> ();

		SetStopAfter (stopSource);
	}

	void Player::RestorePlayState ()
	{
		const auto wasPlaying = XmlSettingsManager::Instance ().Property ("WasPlaying", false).toBool ();
		if (wasPlaying)
			Source_->Play ();

		IgnoreNextSaves_ = false;
	}

	void Player::SavePlayState (bool ignoreNext)
	{
		if (IgnoreNextSaves_)
			return;

		const auto state = Source_->GetState ();
		const auto isPlaying = state == SourceState::Playing ||
				state == SourceState::Buffering;
		XmlSettingsManager::Instance ().setProperty ("WasPlaying", isPlaying);

		IgnoreNextSaves_ = ignoreNext;
	}

	void Player::AddToOneShotQueue (const QModelIndex& index)
	{
		if (index.data (Role::IsAlbum).toBool ())
		{
			for (int i = 0, rc = PlaylistModel_->rowCount (index); i < rc; ++i)
				AddToOneShotQueue (PlaylistModel_->index (i, 0, index));
			return;
		}

		AddToOneShotQueue (index.data (Role::Source).value<AudioSource> ());
	}

	void Player::AddToOneShotQueue (const AudioSource& source)
	{
		if (CurrentOneShotQueue_.contains (source))
			return;

		CurrentOneShotQueue_ << source;

		const auto pos = CurrentOneShotQueue_.size () - 1;

		if (auto item = Items_ [source])
			item->setData (pos, Role::OneShotPos);
	}

	void Player::RemoveFromOneShotQueue (const QModelIndex& index)
	{
		if (index.data (Role::IsAlbum).toBool ())
		{
			for (int i = 0, rc = PlaylistModel_->rowCount (index); i < rc; ++i)
				RemoveFromOneShotQueue (PlaylistModel_->index (i, 0, index));
			return;
		}

		const auto& source = index.data (Role::Source).value<AudioSource> ();
		RemoveFromOneShotQueue (source);
	}

	void Player::OneShotMoveUp (const QModelIndex& index)
	{
		if (index.data (Role::IsAlbum).toBool ())
		{
			for (int i = 0, rc = PlaylistModel_->rowCount (index); i < rc; ++i)
				OneShotMoveUp (PlaylistModel_->index (i, 0, index));
			return;
		}

		const auto& source = index.data (Role::Source).value<AudioSource> ();
		const auto pos = CurrentOneShotQueue_.indexOf (source);
		if (pos <= 0)
			return;

		std::swap (CurrentOneShotQueue_ [pos], CurrentOneShotQueue_ [pos - 1]);
		Items_ [CurrentOneShotQueue_.at (pos)]->setData (pos, Role::OneShotPos);
		Items_ [CurrentOneShotQueue_.at (pos - 1)]->setData (pos - 1, Role::OneShotPos);
	}

	void Player::OneShotMoveDown (const QModelIndex& index)
	{
		if (index.data (Role::IsAlbum).toBool ())
		{
			for (int i = PlaylistModel_->rowCount (index) - 1; i >= 0; --i)
				OneShotMoveDown (PlaylistModel_->index (i, 0, index));
			return;
		}

		const auto& source = index.data (Role::Source).value<AudioSource> ();
		const auto pos = CurrentOneShotQueue_.indexOf (source);
		if (pos == CurrentOneShotQueue_.size () - 1)
			return;

		std::swap (CurrentOneShotQueue_ [pos], CurrentOneShotQueue_ [pos + 1]);
		Items_ [CurrentOneShotQueue_.at (pos)]->setData (pos, Role::OneShotPos);
		Items_ [CurrentOneShotQueue_.at (pos + 1)]->setData (pos + 1, Role::OneShotPos);
	}

	int Player::GetOneShotQueueSize () const
	{
		return CurrentOneShotQueue_.size ();
	}

	void Player::SetRadioStation (Media::IRadioStation_ptr station)
	{
		clear ();

		CurrentStation_ = station;

		connect (CurrentStation_->GetQObject (),
				SIGNAL (gotError (const QString&)),
				this,
				SLOT (handleStationError (const QString&)));
		connect (CurrentStation_->GetQObject (),
				SIGNAL (gotNewStream (QUrl, Media::AudioInfo)),
				this,
				SLOT (handleRadioStream (QUrl, Media::AudioInfo)));
		connect (CurrentStation_->GetQObject (),
				SIGNAL (gotPlaylist (QString, QString)),
				this,
				SLOT (handleGotRadioPlaylist (QString, QString)));
		connect (CurrentStation_->GetQObject (),
				SIGNAL (gotAudioInfos (QList<Media::AudioInfo>)),
				this,
				SLOT (handleGotAudioInfos (QList<Media::AudioInfo>)));
		CurrentStation_->RequestNewStream ();

		auto radioName = station->GetRadioName ();
		if (radioName.isEmpty ())
			radioName = tr ("Radio");
		const auto radioItem = new QStandardItem (radioName);
		radioItem->setEditable (false);
		radioItem->setData (true, Role::IsRadioItem);
		PlaylistModel_->appendRow (radioItem);
	}

	MediaInfo Player::GetCurrentMediaInfo () const
	{
		const auto& source = Source_->GetActualSource ();
		if (source.IsEmpty ())
			return MediaInfo ();

		auto info = GetMediaInfo (source);
		if (!info.LocalPath_.isEmpty ())
			return info;

		info = GetPhononMediaInfo ();
		return info;
	}

	QString Player::GetCurrentAAPath () const
	{
		const auto& info = GetCurrentMediaInfo ();
		auto coll = Core::Instance ().GetLocalCollection ();
		auto album = coll->GetAlbum (coll->FindAlbum (info.Artist_, info.Album_));
		return album ? album->CoverPath_ : QString ();;
	}

	MediaInfo Player::GetMediaInfo (const AudioSource& source) const
	{
		return Items_.contains (source) ?
				Items_ [source]->data (Role::Info).value<MediaInfo> () :
				MediaInfo ();
	}

	NativePlaylist_t Player::GetAsNativePlaylist () const
	{
		const auto& current = Source_->GetCurrentSource ();

		return Util::Map (CurrentQueue_,
				[this, current] (const AudioSource& source)
				{
					std::optional<MediaInfo> info;
					const auto& url = source.ToUrl ();
					if (Url2Info_.contains (url))
						info = Url2Info_ [url];

					if (source == current)
					{
						if (!info)
							info = MediaInfo {};
						info->Additional_ ["Current"] = true;
					}

					return NativePlaylistItem_t { source, info };
				});
	}

	namespace
	{
		struct RestoreInfo
		{
			QString RadioID_;

			QUrl Url_;
			MediaInfo Media_;
		};

		void HandlePluginInfos (const QByteArray& pluginId,
				const QList<RestoreInfo>& infos,
				QFutureSynchronizer<Media::RadiosRestoreResult_t> *syncer,
				IPluginsManager *ipm)
		{
			const auto pObj = ipm->GetPluginByID (pluginId);
			if (!pObj)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot find plugin for"
						<< pluginId
						<< ";"
						<< infos.size ()
						<< "playlist items will be lost :(";
				return;
			}

			const auto irrsp = qobject_cast<Media::IRestorableRadioStationProvider*> (pObj);
			if (!irrsp)
			{
				qWarning () << Q_FUNC_INFO
						<< "plugin"
						<< pObj
						<< "for"
						<< pluginId
						<< "cannot be cast to Media::IRestorableRadioStationProvider;"
						<< infos.size ()
						<< "playlist items will be lost :(";
				return;
			}

			const auto& ids = Util::Map (infos, &RestoreInfo::RadioID_);
			const auto& future = irrsp->RestoreRadioStations (ids);
			if (future.isCanceled ())
			{
				qWarning () << Q_FUNC_INFO
						<< "plugin"
						<< pObj
						<< "for"
						<< pluginId
						<< "returned null future; so"
						<< infos.size ()
						<< "playlist items will be lost in the present :(";
				return;
			}

			syncer->addFuture (future);
		}

		NativePlaylist_t HandleRestored (const NativePlaylist_t& playlist,
				const QHash<QPair<QString, QString>, Media::RadioRestoreVariant_t>& restored)
		{
			std::decay_t<decltype (playlist)> newPlaylist;

			for (const auto& item : playlist)
			{
				if (!item.second)
				{
					newPlaylist << item;
					continue;
				}

				const auto& media = *item.second;

				const auto& pluginID = media.Additional_ ["LMP/PluginID"].toByteArray ();
				const auto& radioID = media.Additional_ ["LMP/RadioID"].toString ();

				if (!restored.contains ({ pluginID, radioID }))
				{
					newPlaylist << item;
					continue;
				}

				auto restoredPlaylist = Util::Visit (restored.value ({ pluginID, radioID }),
						[] (const QList<Media::AudioInfo>& infos)
						{
							return Util::Map (infos,
									[] (const Media::AudioInfo& info)
									{
										return NativePlaylistItem_t
										{
											info.Other_ ["URL"].toUrl (),
											MediaInfo::FromAudioInfo (info)
										};
									});
						});

				for (auto& pair : restoredPlaylist)
					if (pair.second)
					{
						auto& info = *pair.second;
						info.Additional_ ["LMP/RadioID"] = radioID;
						info.Additional_ ["LMP/PluginID"] = pluginID;
					}

				newPlaylist += restoredPlaylist;
			}

			return newPlaylist;
		}

		template<typename UrlInfoSetter, typename Setter, typename Clearer>
		void CheckPlaylistRefreshes (const NativePlaylist_t& playlist,
				const UrlInfoSetter& urlInfoSetter, const Setter& setter, const Clearer& clearer,
				const ICoreProxy_ptr& proxy)
		{
			QHash<QByteArray, QList<RestoreInfo>> plugin2infos;
			for (const auto& item : playlist)
			{
				if (!item.second)
					continue;

				const auto& media = *item.second;

				const auto& pluginID = media.Additional_ ["LMP/PluginID"].toByteArray ();
				const auto& radioID = media.Additional_ ["LMP/RadioID"].toString ();

				urlInfoSetter (item.first.ToUrl (), media);

				if (!radioID.isEmpty () && !pluginID.isEmpty ())
					plugin2infos [pluginID].append ({ radioID, item.first.ToUrl (), media });
			}

			const auto syncer = std::make_shared<QFutureSynchronizer<Media::RadiosRestoreResult_t>> ();

			const auto ipm = proxy->GetPluginsManager ();
			for (const auto& pair : Util::Stlize (plugin2infos))
				HandlePluginInfos (pair.first, pair.second, syncer.get (), ipm);

			if (syncer->futures ().isEmpty ())
				return;

			Util::Sequence (nullptr, QtConcurrent::run ([syncer] { syncer->waitForFinished (); })) >>
					[=]
					{
						QHash<QPair<QString, QString>, Media::RadioRestoreVariant_t> restored;
						for (const auto& future : syncer->futures ())
							for (const auto& item : future.result ())
								restored [{ item.PluginID_, item.RadioID_ }] = item.Restored_;

						const auto& newPlaylist = HandleRestored (playlist, restored);
						if (newPlaylist == playlist)
							return;

						clearer ();

						for (const auto& item : newPlaylist)
							if (item.second)
								urlInfoSetter (item.first.ToUrl (), *item.second);

						setter (newPlaylist);
					};
		}
	}

	void Player::SetNativePlaylist (NativePlaylist_t playlist)
	{
		playlist = GetAsNativePlaylist () + playlist;

		auto setter = [this] (const NativePlaylist_t& pl)
		{
			Enqueue (Util::Map (pl, &NativePlaylistItem_t::first));
		};

		CheckPlaylistRefreshes (playlist,
				[this] (const QUrl& url, const MediaInfo& media) { Url2Info_ [url] = media; },
				setter,
				[this] { clear (); },
				Proxy_);

		setter (playlist);
	}

	MediaInfo Player::GetPhononMediaInfo () const
	{
		MediaInfo info;
		info.Artist_ = Source_->GetMetadata (SourceObject::Metadata::Artist);
		info.Album_ = Source_->GetMetadata (SourceObject::Metadata::Album);
		info.Title_ = Source_->GetMetadata (SourceObject::Metadata::Title);
		info.Genres_ << Source_->GetMetadata (SourceObject::Metadata::Genre);
		info.TrackNumber_ = Source_->GetMetadata (SourceObject::Metadata::Tracknumber).toInt ();
		info.Length_ = Source_->GetTotalTime () / 1000;
		info.LocalPath_ = Source_->GetActualSource ().ToUrl ().toString ();

		if (info.Artist_.isEmpty () && info.Title_.contains (" - "))
		{
			const auto& strs = info.Title_.split (" - ", Qt::SkipEmptyParts);
			switch (strs.size ())
			{
			case 2:
				info.Artist_ = strs.value (0);
				info.Title_ = strs.value (1);
				break;
			case 3:
				info.Artist_ = strs.value (0);
				info.Album_ = strs.value (1);
				info.Title_ = strs.value (2);
				break;
			}
		}

		auto append = [&info, this] (SourceObject::Metadata md, const QString& name) -> void
		{
			const auto& value = Source_->GetMetadata (md);
			if (!value.isEmpty ())
				info.Additional_ [name] = value;
		};

		append (SourceObject::Metadata::NominalBitrate, tr ("Bitrate"));
		append (SourceObject::Metadata::MinBitrate, tr ("Minimum bitrate"));
		append (SourceObject::Metadata::MaxBitrate, tr ("Maximum bitrate"));

		return info;
	}

	namespace
	{
		QStandardItem* MakeAlbumItem (const MediaInfo& info)
		{
			auto albumItem = new QStandardItem (QString ("%1 - %2")
					.arg (info.Artist_, info.Album_));
			albumItem->setEditable (false);
			albumItem->setData (true, Player::Role::IsAlbum);
			albumItem->setData (QVariant::fromValue (info), Player::Role::Info);
			albumItem->setData (0, Player::Role::AlbumLength);
			return albumItem;
		}

		void LoadAlbumArt (QStandardItem *albumItem, const MediaInfo& info)
		{
			const int dim = 48;
			const auto worker = [info]
			{
				auto artImage = FindAlbumArt<QImage> (info.LocalPath_);
				if (std::max (artImage.width (), artImage.height ()) > dim)
					artImage = artImage.scaled (dim, dim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				return artImage;
			};

			const QPersistentModelIndex guardIdx { albumItem->index () };
			const auto futureWatcher = new QFutureWatcher<QImage>;
			new Util::SlotClosure<Util::DeleteLaterPolicy>
			{
				[albumItem, futureWatcher, guardIdx]
				{
					if (!guardIdx.isValid ())
						return;

					const auto& artImage = futureWatcher->result ();
					auto art = QPixmap::fromImage (artImage);
					if (art.isNull ())
						art = QIcon::fromTheme ("media-optical").pixmap (dim, dim);
					albumItem->setData (art, Player::Role::AlbumArt);
					futureWatcher->deleteLater ();
				},
				futureWatcher,
				SIGNAL (finished ()),
				futureWatcher
			};
			futureWatcher->setFuture (QtConcurrent::run (worker));
		}

		using ResolvedSource_t = QPair<AudioSource, MediaInfo>;

		template<typename NonLocalGetter>
		ResolvedSource_t PairResolve (const NonLocalGetter& getter, const AudioSource& source)
		{
			if (!source.IsLocalFile ())
				return { source, getter (source) };

			MediaInfo info;
			info.LocalPath_ = source.GetLocalPath ();

			auto collection = Core::Instance ().GetLocalCollection ();

			const auto trackId = collection->FindTrack (source.GetLocalPath ());
			if (trackId == -1)
			{
				auto resolver = Core::Instance ().GetLocalFileResolver ();
				return Util::Visit (resolver->ResolveInfo (source.GetLocalPath ()),
						[&source] (const MediaInfo& resolved) { return ResolvedSource_t { source, resolved }; },
						[&source, &info] (const ResolveError&)
						{
							qWarning () << Q_FUNC_INFO
									<< "could not find track"
									<< info.LocalPath_
									<< "in library and cannot resolve its info, probably missing?";
							return ResolvedSource_t { source, info };
						});
			}

			info.Artist_ = collection->GetTrackData (trackId,
					LocalCollectionModel::Role::ArtistName).toString ();
			info.Album_ = collection->GetTrackData (trackId,
					LocalCollectionModel::Role::AlbumName).toString ();
			info.Title_ = collection->GetTrackData (trackId,
					LocalCollectionModel::Role::TrackTitle).toString ();
			info.Genres_ = collection->GetTrackData (trackId,
					LocalCollectionModel::Role::TrackGenres).toStringList ();
			info.Length_ = collection->GetTrackData (trackId,
					LocalCollectionModel::Role::TrackLength).toInt ();
			info.Year_ = collection->GetTrackData (trackId,
					LocalCollectionModel::Role::AlbumYear).toInt ();
			info.TrackNumber_ = collection->GetTrackData (trackId,
					LocalCollectionModel::Role::TrackNumber).toInt ();

			return { source, info };
		}

		template<typename NonLocalGetter>
		ResolveResult_t PairResolveAll (const QList<AudioSource>& sources,
				const NonLocalGetter& getter)
		{
			return Util::Map (sources,
					[&] (const AudioSource& source) { return PairResolve (getter, source); });
		}

		template<typename Sorter, typename NonLocalGetter>
		ResolveResult_t PairResolveSort (const QList<AudioSource>& sources,
				Sorter sorter, NonLocalGetter nonLocalGetter, bool sort)
		{
			auto result = PairResolveAll (sources, nonLocalGetter);

			if (sorter.Criteria_.isEmpty () || !sort)
				return result;

			std::sort (result.begin (), result.end (),
					[sorter] (const ResolvedSource_t& s1, const ResolvedSource_t& s2)
					{
						const auto leftUseful = !s1.second.IsUseless ();
						const auto rightUseful = !s2.second.IsUseless ();

						if (leftUseful && !rightUseful)
							return true;
						else if (!leftUseful && rightUseful)
							return false;
						else if (!leftUseful || !rightUseful)
							return s1.first.ToUrl () < s2.first.ToUrl ();
						else
							return sorter (s1.second, s2.second);
					});

			return result;
		}
	}

	void Player::AddToPlaylistModel (QList<AudioSource> sources, bool sort, bool clear)
	{
		if (!CurrentQueue_.isEmpty () && !clear)
		{
			EnqueueFlags flags { EnqueueReplace };
			if (sort)
				flags |= EnqueueSort;
			Enqueue (CurrentQueue_ + sources, flags);
			return;
		}

		emit playerAvailable (false);

		const auto future = QtConcurrent::run ([=, this]
				{
					return ResolveJobResult
					{
						PairResolveSort (sources,
								Sorter_,
								[this] (const AudioSource& source)
								{
									return Url2Info_.value (source.ToUrl ());
								},
								sort),
						clear
					};
				});
		Util::Sequence (this, future) >>
				[this] (const ResolveJobResult& result)
				{
					ContinueAfterSorted (result);
					emit playerAvailable (true);
				};
	}

	void Player::SetStopAfter (const AudioSource& stopSource)
	{
		if (!CurrentStopSource_.IsEmpty ())
			Items_ [CurrentStopSource_]->setData (false, Role::IsStop);

		if (CurrentStopSource_ == stopSource)
			CurrentStopSource_ = AudioSource ();
		else
		{
			CurrentStopSource_ = stopSource;
			Items_ [stopSource]->setData (true, Role::IsStop);
		}

		emit currentStopSourceChanged ();
	}

	bool Player::HandleCurrentStop (const AudioSource& source)
	{
		if (source != CurrentStopSource_)
			return false;

		CurrentStopSource_ = AudioSource ();
		Items_ [source]->setData (false, Role::IsStop);

		return true;
	}

	void Player::RemoveFromOneShotQueue (const AudioSource& source)
	{
		const auto pos = CurrentOneShotQueue_.indexOf (source);
		if (pos < 0)
			return;

		CurrentOneShotQueue_.removeAt (pos);
		for (int i = pos; i < CurrentOneShotQueue_.size (); ++i)
			Items_ [CurrentOneShotQueue_.at (i)]->setData (i, Role::OneShotPos);

		Items_ [source]->setData ({}, Role::OneShotPos);
	}

	QStandardItem* Player::FindRadioItem () const
	{
		for (int i = 0; i < PlaylistModel_->rowCount (); ++i)
		{
			const auto item = PlaylistModel_->item (i);
			if (item->data (Role::IsRadioItem).toBool ())
				return item;
		}

		return nullptr;
	}

	void Player::UnsetRadio ()
	{
		if (!CurrentStation_)
			return;

		if (const auto item = FindRadioItem ())
			PlaylistModel_->removeRow (item->row ());

		CurrentStation_.reset ();
	}

	void Player::EmitStateChange (SourceState state)
	{
		QString stateStr;
		QString hrStateStr;
		switch (state)
		{
		case SourceState::Paused:
			stateStr = "Paused";
			hrStateStr = tr ("paused");
			break;
		case SourceState::Buffering:
		case SourceState::Playing:
			stateStr = "Playing";
			hrStateStr = tr ("playing");
			break;
		default:
			stateStr = "Stopped";
			hrStateStr = tr ("stopped");
			break;
		}

		const auto& mediaInfo = GetCurrentMediaInfo ();
		const auto& str = tr ("%1 by %2 is now %3")
				.arg (mediaInfo.Title_)
				.arg (mediaInfo.Artist_)
				.arg (hrStateStr);

		auto e = Util::MakeAN ("LMP", str, Priority::Info,
				"org.LeechCraft.LMP", AN::CatMediaPlayer, AN::TypeMediaPlaybackStatus,
				"org.LeechCraft.LMP.PlaybackStatus", {}, 0, 1, str);
		e.Mime_ += "+advanced";
		e.Additional_ [AN::Field::MediaPlaybackStatus] = stateStr;
		e.Additional_ [AN::Field::MediaPlayerURL] = Source_->GetActualSource ().ToUrl ();

		e.Additional_ [AN::Field::MediaArtist] = mediaInfo.Artist_;
		e.Additional_ [AN::Field::MediaAlbum] = mediaInfo.Album_;
		e.Additional_ [AN::Field::MediaTitle] = mediaInfo.Title_;
		e.Additional_ [AN::Field::MediaLength] = mediaInfo.Length_;

		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	template<typename T>
	AudioSource Player::GetRandomBy (AudioSources_t::const_iterator pos,
			std::function<T (AudioSources_t::const_iterator, AudioSources_t)> feature) const
	{
		auto randPos = [&feature, this] (const QList<AudioSource>& sources) -> int
		{
			const auto begin = sources.begin ();
			QHash<T, QList<int>> fVals;
			for (auto i = begin, end = sources.end (); i != end; ++i)
				fVals [feature (i, sources)] << std::distance (begin, i);

			auto fIdx = std::uniform_int_distribution<int> (0, fVals.size () - 1) (PRG_);

			auto fPos = fVals.begin ();
			std::advance (fPos, fIdx);
			const auto& positions = *fPos;
			if (positions.size () < 2)
				return positions [0];

			auto posIdx = std::uniform_int_distribution<int> (0, positions.size () - 1) (PRG_);
			return positions [posIdx];
		};
		auto rand = [&randPos] (const QList<AudioSource>& sources)
			{ return sources.at (randPos (sources)); };

		if (pos == CurrentQueue_.end ())
			return rand (CurrentQueue_);

		const auto& current = feature (pos, CurrentQueue_);
		++pos;
		if (pos != CurrentQueue_.end () && feature (pos, CurrentQueue_) == current)
			return *pos;

		decltype (CurrentQueue_) modifiedQueue;
		for (auto i = CurrentQueue_.begin (); i != CurrentQueue_.end (); ++i)
			if (feature (i, CurrentQueue_) != current)
				modifiedQueue << *i;
		if (modifiedQueue.isEmpty ())
			return rand (CurrentQueue_);

		const auto& origFeature = feature (pos, modifiedQueue);
		pos = modifiedQueue.begin () + randPos (modifiedQueue);
		while (pos != modifiedQueue.begin ())
		{
			if (feature (pos - 1, modifiedQueue) != origFeature)
				break;
			--pos;
		}
		return *pos;
	}

	AudioSource Player::GetNextSource (const AudioSource& current)
	{
		if (CurrentQueue_.isEmpty ())
			return {};

		if (!CurrentOneShotQueue_.isEmpty ())
		{
			const auto first = CurrentOneShotQueue_.front ();
			RemoveFromOneShotQueue (first);
			return first;
		}

		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);

		switch (PlayMode_)
		{
		case PlayMode::Sequential:
			if (pos == CurrentQueue_.end ())
				return CurrentQueue_.value (0);
			else if (++pos != CurrentQueue_.end ())
				return *pos;
			else
				return {};
		case PlayMode::Shuffle:
			return GetRandomBy<int> (pos,
					[] (AudioSources_t::const_iterator pos, const AudioSources_t& sources)
						{ return pos - sources.begin (); });
		case PlayMode::ShuffleAlbums:
			return GetRandomBy<QString> (pos,
					[this] (AudioSources_t::const_iterator pos, const AudioSources_t&)
						{ return GetMediaInfo (*pos).Album_; });
		case PlayMode::ShuffleArtists:
			return GetRandomBy<QString> (pos,
					[this] (AudioSources_t::const_iterator pos, const AudioSources_t&)
						{ return GetMediaInfo (*pos).Artist_; });
		case PlayMode::RepeatTrack:
			return current;
		case PlayMode::RepeatAlbum:
		{
			if (pos == CurrentQueue_.end ())
				return CurrentQueue_.value (0);

			const auto& curAlbum = GetMediaInfo (*pos).Album_;
			++pos;
			if (pos == CurrentQueue_.end () ||
					GetMediaInfo (*pos).Album_ != curAlbum)
			{
				do
					--pos;
				while (pos >= CurrentQueue_.begin () &&
						GetMediaInfo (*pos).Album_ == curAlbum);
				++pos;
			}
			return *pos;
		}
		case PlayMode::RepeatWhole:
			if (pos == CurrentQueue_.end () || ++pos == CurrentQueue_.end ())
				pos = CurrentQueue_.begin ();
			return *pos;
		}

		return {};
	}

	void Player::MarkAsCurrent (QStandardItem *curItem)
	{
		if (curItem)
			curItem->setData (true, Role::IsCurrent);
		for (auto item : Items_)
		{
			if (item == curItem)
				continue;
			if (item->data (Role::IsCurrent).toBool ())
			{
				item->setData (false, Role::IsCurrent);
				break;
			}
		}
	}

	void Player::play (const QModelIndex& index)
	{
		if (CurrentStation_)
		{
			if (PlaylistModel_->itemFromIndex (index)->data (Role::IsRadioItem).toBool ())
				return;
			else
				UnsetRadio ();
		}

		if (index.data (Role::IsAlbum).toBool ())
		{
			play (PlaylistModel_->index (0, 0, index));
			return;
		}

		if (!index.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index;
			return;
		}

		if (Source_->GetState () != SourceState::Stopped)
			emit aboutToStopInternally ();

		Source_->Stop ();
		Source_->ClearQueue ();
		const auto& source = index.data (Role::Source).value<AudioSource> ();
		Source_->SetCurrentSource (source);
		Source_->Play ();
	}

	void Player::previousTrack ()
	{
		const auto& current = Source_->GetCurrentSource ();

		AudioSource next;
		if (PlayMode_ == PlayMode::Shuffle)
		{
			next = GetNextSource (current);
			if (next.IsEmpty ())
				return;
		}
		else
		{
			const auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
			if (pos == CurrentQueue_.begin ())
				return;

			next = pos == CurrentQueue_.end () ? CurrentQueue_.value (0) : *(pos - 1);
		}

		if (Source_->GetState () != SourceState::Stopped)
			emit aboutToStopInternally ();
		Source_->Stop ();
		Source_->SetCurrentSource (next);
		Source_->Play ();
	}

	void Player::nextTrack ()
	{
		if (CurrentStation_)
		{
			Source_->Clear ();
			CurrentStation_->RequestNewStream ();
			return;
		}

		const auto& current = Source_->GetCurrentSource ();
		const auto& next = GetNextSource (current);
		if (next.IsEmpty ())
			return;

		if (Source_->GetState () != SourceState::Stopped)
			emit aboutToStopInternally ();
		Source_->Stop ();
		Source_->SetCurrentSource (next);
		Source_->Play ();
	}

	void Player::togglePause ()
	{
		if (Source_->GetState () == SourceState::Playing)
			Source_->Pause ();
		else
		{
			const auto& current = Source_->GetCurrentSource ();
			if (current.IsEmpty ())
				Source_->SetCurrentSource (CurrentQueue_.value (0));
			Source_->Play ();
		}
	}

	void Player::setPause ()
	{
		Source_->Pause ();
	}

	void Player::stop ()
	{
		Source_->Stop ();

		if (CurrentStation_)
			UnsetRadio ();
	}

	void Player::stopAfterCurrent ()
	{
		SetStopAfter (Source_->GetActualSource ());
	}

	void Player::clear ()
	{
		UnsetRadio ();

		if (const auto rc = PlaylistModel_->rowCount ())
			PlaylistModel_->removeRows (0, rc);

		Items_.clear ();
		AlbumRoots_.clear ();
		CurrentQueue_.clear ();
		Url2Info_.clear ();
		CurrentOneShotQueue_.clear ();
		Source_->ClearQueue ();

		XmlSettingsManager::Instance ().setProperty ("LastSong", QString ());

		SaveOnLoadPlaylist ();

		if (Source_->GetState () != SourceState::Playing)
			Source_->SetCurrentSource ({});
	}

	void Player::shufflePlaylist ()
	{
		SetPlayMode (PlayMode::Sequential);

		auto queue = GetQueue ();
		std::shuffle (queue.begin (), queue.end (), std::mt19937 { std::random_device {} () });
		Enqueue (queue, EnqueueReplace);
	}

	namespace
	{
		void FillItem (QStandardItem *item, const MediaInfo& info)
		{
			const auto& text = !info.IsUseless () ?
					PerformSubstitutionsPlaylist (info) :
					QFileInfo (info.LocalPath_).fileName ();
			item->setText (text);
			item->setData (QVariant::fromValue (info), Player::Role::Info);
		}
	}

	void Player::ContinueAfterSorted (const ResolveJobResult& result)
	{
		const auto& sources = result.Resolved_;

		CurrentQueue_.clear ();

		QMetaObject::invokeMethod (PlaylistModel_, "modelAboutToBeReset");

		if (result.ShouldClear_)
		{
			if (const auto rc = PlaylistModel_->rowCount ())
				PlaylistModel_->removeRows (0, rc);
			Items_.clear ();
			AlbumRoots_.clear ();
		}

		PlaylistModel_->blockSignals (true);

		QString prevAlbumRoot;

		QList<QStandardItem*> managedRulesItems;

		for (const auto& sourcePair : sources)
		{
			const auto& source = sourcePair.first;
			CurrentQueue_ << source;

			auto item = new QStandardItem ();
			item->setEditable (false);
			item->setData (QVariant::fromValue (source), Role::Source);
			item->setData (source == CurrentStopSource_, Role::IsStop);

			const auto oneShotPos = CurrentOneShotQueue_.indexOf (source);
			if (oneShotPos >= 0)
				item->setData (oneShotPos, Role::OneShotPos);

			switch (source.GetType ())
			{
			case AudioSource::Type::Stream:
				item->setText (tr ("Stream"));
				PlaylistModel_->appendRow (item);
				break;
			case AudioSource::Type::Url:
			{
				const auto& url = source.ToUrl ();

				auto info = Core::Instance ().TryURLResolve (url);
				if (!info && Url2Info_.contains (url))
					info = Url2Info_ [url];

				if (info)
					FillItem (item, *info);
				else
					item->setText (url.toString ());

				PlaylistModel_->appendRow (item);
				break;
			}
			case AudioSource::Type::File:
			{
				const auto& info = sourcePair.second;

				managedRulesItems << item;

				const auto& albumID = info.Album_;
				FillItem (item, info);
				if (albumID != prevAlbumRoot ||
						AlbumRoots_ [albumID].isEmpty ())
				{
					PlaylistModel_->appendRow (item);

					if (!info.Album_.simplified ().isEmpty ())
						AlbumRoots_ [albumID] << item;
				}
				else if (AlbumRoots_ [albumID].last ()->data (Role::IsAlbum).toBool ())
				{
					IncAlbumLength (AlbumRoots_ [albumID].last (), info.Length_);
					AlbumRoots_ [albumID].last ()->appendRow (item);
				}
				else
				{
					auto albumItem = MakeAlbumItem (info);

					const int row = AlbumRoots_ [albumID].last ()->row ();
					const auto& existing = PlaylistModel_->takeRow (row);
					albumItem->appendRow (existing);
					albumItem->appendRow (item);
					PlaylistModel_->insertRow (row, albumItem);

					LoadAlbumArt (albumItem, info);

					const auto& existingInfo = existing.at (0)->data (Role::Info).value<MediaInfo> ();
					albumItem->setData (existingInfo.Length_, Role::AlbumLength);
					IncAlbumLength (albumItem, info.Length_);

					emit insertedAlbum (albumItem->index ());

					AlbumRoots_ [albumID].last () = albumItem;
				}
				prevAlbumRoot = albumID;
				break;
			}
			default:
				item->setText ("unknown");
				PlaylistModel_->appendRow (item);
				break;
			}

			Items_ [source] = item;
		}

		PlaylistModel_->blockSignals (false);

		QMetaObject::invokeMethod (PlaylistModel_, "modelReset");

		SaveOnLoadPlaylist ();

		if (Source_->GetState () == SourceState::Stopped)
		{
			const auto& songUrl = XmlSettingsManager::Instance ().property ("LastSong").toByteArray ();
			const auto& song = QUrl::fromEncoded (songUrl);
			if (!song.isEmpty ())
			{
				const auto pos = std::find_if (CurrentQueue_.begin (), CurrentQueue_.end (),
						[&song] (const auto& item) { return song == item.ToUrl (); });
				if (pos != CurrentQueue_.end ())
					Source_->SetCurrentSource (*pos);
			}

			if (FirstPlaylistRestore_ &&
					XmlSettingsManager::Instance ().property ("AutoContinuePlayback").toBool ())
				RestorePlayState ();
			FirstPlaylistRestore_ = false;
		}

		const auto& currentSource = Source_->GetCurrentSource ();
		if (Items_.contains (currentSource))
			Items_ [currentSource]->setData (true, Role::IsCurrent);
	}

	void Player::SaveOnLoadPlaylist () const
	{
		Core::Instance ().GetPlaylistManager ()->
				GetStaticManager ()->SetOnLoadPlaylist (GetAsNativePlaylist ());
	}

	void Player::restorePlaylist ()
	{
		const auto staticMgr = Core::Instance ().GetPlaylistManager ()->GetStaticManager ();
		SetNativePlaylist (staticMgr->GetOnLoadPlaylist ());
		emit playlistRestored ();
	}

	void Player::handleStationError (const QString& error)
	{
		const auto& e = Util::MakeNotification ("LMP",
				tr ("Radio station error: %1.")
					.arg (error),
				Priority::Critical);
		Core::Instance ().SendEntity (e);
	}

	void Player::handleRadioStream (const QUrl& url, const Media::AudioInfo& info)
	{
		Url2Info_ [url] = info;
		Source_->SetCurrentSource (url);

		qDebug () << Q_FUNC_INFO << static_cast<int> (Source_->GetState ());
		if (Source_->GetState () == SourceState::Stopped)
			Source_->Play ();
	}

	void Player::handleGotRadioPlaylist (const QString& name, const QString& format)
	{
		QMetaObject::invokeMethod (this,
				"postPlaylistCleanup",
				Qt::QueuedConnection,
				Q_ARG (QString, name));

		auto parser = MakePlaylistParser (format);
		if (!parser)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find parser for format"
					<< format;
			return;
		}

		const auto& list = parser (name).ToSources ();
		Enqueue (list, EnqueueNone);
	}

	void Player::handleGotAudioInfos (const QList<Media::AudioInfo>& infos)
	{
		QList<AudioSource> sources;
		for (const auto& info : infos)
		{
			const auto& url = info.Other_ ["URL"].toUrl ();
			if (!url.isValid ())
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping invalid URL";
				continue;
			}

			Url2Info_ [url] = info;
			sources << url;
		}

		if (!sources.isEmpty ())
			Enqueue (sources, EnqueueNone);
	}

	void Player::postPlaylistCleanup (const QString& filename)
	{
		UnsetRadio ();
		QFile::remove (filename);
	}

	void Player::handleUpdateSourceQueue (const std::shared_ptr<std::atomic_bool>& isTimeout)
	{
		const auto& current = Source_->GetCurrentSource ();

		if (CurrentStation_)
		{
			Url2Info_.remove (current.ToUrl ());
			CurrentStation_->RequestNewStream ();
			return;
		}

		const auto& path = current.GetLocalPath ();
		if (!path.isEmpty ())
			QTimer::singleShot (0, [=] { Core::Instance ().GetLocalCollection ()->RecordPlayedTrack (path); });

		const auto& next = GetNextSource (current);

		if (HandleCurrentStop (current))
		{
			PlaybackStopHandler_ = [this, next]
			{
				MarkAsCurrent (Items_.value (next));
				Source_->SetCurrentSource (next);
			};
			return;
		}

		if (next.IsEmpty ())
		{
			PlaybackStopHandler_ = [this]
			{
				MarkAsCurrent (nullptr);
				Source_->SetCurrentSource ({});
			};
			return;
		}

		Source_->PrepareNextSource (next);
		EmitStateChange (SourceState::Stopped);

		if (*isTimeout)
		{
			qWarning () << Q_FUNC_INFO
					<< "timeout detected, scheduling playback restart";

			new Util::SlotClosure<Util::DeleteLaterPolicy>
			{
				[this, next]
				{
					Source_->SetCurrentSource (next);
					Source_->Play ();
				},
				Source_,
				SIGNAL (stateChanged (SourceState, SourceState)),
				Source_
			};
		}
	}

	void Player::handlePlaybackFinished ()
	{
		emit songChanged (MediaInfo ());
		Source_->SetCurrentSource ({});
	}

	void Player::handleStateChanged (SourceState state, SourceState oldState)
	{
		qDebug () << Q_FUNC_INFO << static_cast<int> (state) << static_cast<int> (oldState);
		switch (state)
		{
		case SourceState::Stopped:
			emit songChanged ({});

			if (!CurrentQueue_.contains (Source_->GetCurrentSource ()))
				Source_->SetCurrentSource ({});

			if (PlaybackStopHandler_)
			{
				PlaybackStopHandler_ ();
				PlaybackStopHandler_ = nullptr;
			}

			break;
		default:
			break;
		}

		SavePlayState (false);

		EmitStateChange (state);
	}

	void Player::handleCurrentSourceChanged (const AudioSource& source)
	{
		XmlSettingsManager::Instance ().setProperty ("LastSong", source.ToUrl ().toEncoded ());

		QStandardItem *curItem = nullptr;
		if (CurrentStation_)
			curItem = FindRadioItem ();
		else if (Items_.contains (source))
			curItem = Items_ [source];

		if (Url2Info_.contains (source.ToUrl ()))
		{
			const auto& info = Url2Info_ [source.ToUrl ()];
			emit songChanged (info);
		}
		else if (curItem)
			emit songChanged (curItem->data (Role::Info).value<MediaInfo> ());
		else
			emit songChanged (MediaInfo ());

		if (curItem)
			emit indexChanged (PlaylistModel_->indexFromItem (curItem));

		MarkAsCurrent (curItem);

		handleMetadata ();

		const auto sourceState = Source_->GetState ();
		if (sourceState != SourceState::Stopped)
			EmitStateChange (sourceState);
	}

	void Player::handleMetadata ()
	{
		const auto& source = Source_->GetCurrentSource ();
		if (!source.IsRemote () ||
				CurrentStation_ ||
				!Items_.contains (source))
			return;

		auto curItem = Items_ [source];

		const auto& info = GetPhononMediaInfo ();

		if (info.Album_ == LastPhononMediaInfo_.Album_ &&
				info.Artist_ == LastPhononMediaInfo_.Artist_ &&
				info.Title_ == LastPhononMediaInfo_.Title_)
			emit songInfoUpdated (info);
		else
		{
			FillItem (curItem, info);
			emit songChanged (info);
		}

		LastPhononMediaInfo_ = info;

		EmitStateChange (Source_->GetState ());
	}

	void Player::refillPlaylist ()
	{
		Enqueue (GetQueue (), EnqueueReplace);
	}
}
}
