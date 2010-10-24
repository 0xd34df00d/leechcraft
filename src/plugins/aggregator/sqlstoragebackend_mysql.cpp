/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "sqlstoragebackend_mysql.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSqlError>
#include <QVariant>
#include <QSqlRecord>
#include "plugininterface/dblock.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			SQLStorageBackendMysql::SQLStorageBackendMysql (StorageBackend::Type t)
			: Type_ (t)
			{
				DB_ = QSqlDatabase::addDatabase ("QMYSQL", "AggregatorConnection");
				DB_.setDatabaseName (XmlSettingsManager::Instance ()->
						property ("MysqlDBName").toString ());
				DB_.setHostName (XmlSettingsManager::Instance ()->
						property ("MysqlHostname").toString ());
				DB_.setPort (XmlSettingsManager::Instance ()->
						property ("MysqlPort").toInt ());
				DB_.setUserName (XmlSettingsManager::Instance ()->
						property ("MysqlUsername").toString ());
				DB_.setPassword (XmlSettingsManager::Instance ()->
						property ("MysqlPassword").toString ());

				if (!DB_.open ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (DB_.lastError ());
					throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
								.arg (DB_.lastError ().text ())));
				}

				InitializeTables ();
			}

			SQLStorageBackendMysql::~SQLStorageBackendMysql ()
			{
			}

			void SQLStorageBackendMysql::Prepare ()
			{
				//TODO set collation
				FeedFinderByURL_ = QSqlQuery (DB_);
				FeedFinderByURL_.prepare (StorageBackend::LoadQuery ("mysql", "FeedFinderByUrl_query"));

				FeedGetter_ = QSqlQuery (DB_);
				FeedGetter_.prepare (StorageBackend::LoadQuery ("mysql", "FeedGetter_query"));

				FeedSettingsGetter_ = QSqlQuery (DB_);
				FeedSettingsGetter_.prepare (StorageBackend::LoadQuery ("mysql", "FeedSettingsGetter_query"));

				FeedSettingsSetter_ = QSqlQuery (DB_);
				FeedSettingsSetter_.prepare (StorageBackend::LoadQuery ("mysql", "FeedSettingsSetter_query"));

				ChannelsShortSelector_ = QSqlQuery (DB_);
				ChannelsShortSelector_.prepare (StorageBackend::LoadQuery ("mysql", "ChannelsShortSelector_query"));
				ChannelsFullSelector_ = QSqlQuery (DB_);
				ChannelsFullSelector_.prepare (StorageBackend::LoadQuery ("mysql", "ChannelsFullSelector_query"));

				UnreadItemsCounter_ = QSqlQuery (DB_);
				UnreadItemsCounter_.prepare (StorageBackend::LoadQuery ("mysql", "UnreadItemsCounter_query"));

				ItemsShortSelector_ = QSqlQuery (DB_);
				ItemsShortSelector_.prepare (StorageBackend::LoadQuery ("mysql", "ItemsShortSelector_query"));

				ItemFullSelector_ = QSqlQuery (DB_);
				ItemFullSelector_.prepare (StorageBackend::LoadQuery ("mysql", "ItemFullSelector_query"));

				ChannelFinder_ = QSqlQuery (DB_);
				ChannelFinder_.prepare (StorageBackend::LoadQuery ("mysql", "ChannelFinder_query"));

				ChannelIDFromTitleURL_ = QSqlQuery (DB_);
				ChannelIDFromTitleURL_.prepare (StorageBackend::LoadQuery ("mysql", "ChannelIDFromTitle_query"));

				ItemIDFromTitleURL_ = QSqlQuery (DB_);
				ItemIDFromTitleURL_.prepare (StorageBackend::LoadQuery ("mysql", "ItemIDFromTitleURL_query"));

				InsertFeed_ = QSqlQuery (DB_);
				InsertFeed_.prepare (StorageBackend::LoadQuery ("mysql", "InsertFeed_query"));

				InsertChannel_ = QSqlQuery (DB_);
				InsertChannel_.prepare (StorageBackend::LoadQuery ("mysql", "InsertChannel_query"));

				InsertItem_ = QSqlQuery (DB_);
				InsertItem_.prepare (StorageBackend::LoadQuery ("mysql", "InsertItem_query"));

				UpdateShortChannel_ = QSqlQuery (DB_);
				UpdateShortChannel_.prepare (StorageBackend::LoadQuery ("mysql", "UpdateShortChannel_query" ));

				UpdateChannel_ = QSqlQuery (DB_);
				UpdateChannel_.prepare (StorageBackend::LoadQuery ("mysql", "UpdateChannel_query"));

				ChannelDateTrimmer_ = QSqlQuery (DB_);
				ChannelDateTrimmer_.prepare (StorageBackend::LoadQuery ("mysql", "ChannelDateTrimmer_query"));

				ChannelNumberTrimmer_ = QSqlQuery (DB_);
				ChannelNumberTrimmer_.prepare (StorageBackend::LoadQuery ("mysql", "ChannelNumberTrimmer_query"));

				UpdateShortItem_ = QSqlQuery (DB_);
				UpdateShortItem_.prepare (StorageBackend::LoadQuery ("mysql", "UpdateShortItem_query"));

				UpdateItem_ = QSqlQuery (DB_);
				UpdateItem_.prepare (StorageBackend::LoadQuery ("mysql", "UpdateItem_query"));

				ToggleChannelUnread_ = QSqlQuery (DB_);
				ToggleChannelUnread_.prepare (StorageBackend::LoadQuery ("mysql", "ToggleChannelUnread_query"));

				RemoveFeed_ = QSqlQuery (DB_);
				RemoveFeed_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveFeed_query"));

				RemoveChannel_ = QSqlQuery (DB_);
				RemoveChannel_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveChannel_query"));

				RemoveItem_ = QSqlQuery (DB_);
				RemoveItem_.prepare (StorageBackend::LoadQuery ("mysql","RemoveChannel_query"));

				WriteEnclosure_ = QSqlQuery (DB_);
				WriteEnclosure_.prepare (StorageBackend::LoadQuery ("mysql", "WriteEnclosure_query"));

				WriteMediaRSS_ = QSqlQuery (DB_);
				WriteMediaRSS_.prepare (StorageBackend::LoadQuery ("mysql", "WriteMediaRSS_query"));

				GetMediaRSSs_ = QSqlQuery (DB_);
				GetMediaRSSs_.prepare (StorageBackend::LoadQuery ("mysql", "GetMediaRSSs_query"));

				WriteMediaRSSThumbnail_ = QSqlQuery (DB_);
				WriteMediaRSSThumbnail_.prepare (StorageBackend::LoadQuery ("mysql", "WriteMediaRSSThumbnail_query"));

				GetMediaRSSThumbnails_ = QSqlQuery (DB_);
				GetMediaRSSThumbnails_.prepare (StorageBackend::LoadQuery ("mysql", "GetMediaRSSThumbnails_query"));

				WriteMediaRSSCredit_ = QSqlQuery (DB_);
				WriteMediaRSSCredit_.prepare (StorageBackend::LoadQuery ("mysql", "WriteMediaRSSCredit_query"));

				GetMediaRSSCredits_ = QSqlQuery (DB_);
				GetMediaRSSCredits_.prepare (StorageBackend::LoadQuery ("mysql", "GetMediaRSSCredits_query"));

				WriteMediaRSSComment_ = QSqlQuery (DB_);
				WriteMediaRSSComment_.prepare (StorageBackend::LoadQuery ("mysql", "WriteMediaRSSComment_query"));

				GetMediaRSSComments_ = QSqlQuery (DB_);
				GetMediaRSSComments_.prepare (StorageBackend::LoadQuery ("mysql", "WriteMediaRSSComment_query"));

				WriteMediaRSSPeerLink_ = QSqlQuery (DB_);
				WriteMediaRSSPeerLink_.prepare (StorageBackend::LoadQuery ("mysql", "WriteMediaRSSPeerLink_query"));

				GetMediaRSSPeerLinks_ = QSqlQuery (DB_);
				GetMediaRSSPeerLinks_.prepare (StorageBackend::LoadQuery ("mysql", "GetMediaRSSPeerLinks_query"));

				WriteMediaRSSScene_ = QSqlQuery (DB_);
				WriteMediaRSSScene_.prepare (StorageBackend::LoadQuery ("mysql", "WriteMediaRSSScene_query"));

				GetMediaRSSScenes_ = QSqlQuery (DB_);
				GetMediaRSSScenes_.prepare (StorageBackend::LoadQuery ("mysql", "GetMediaRSSScenes_query"));

				RemoveEnclosures_ = QSqlQuery (DB_);
				RemoveEnclosures_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveEnclosures_query"));

				GetEnclosures_ = QSqlQuery (DB_);
				GetEnclosures_.prepare (StorageBackend::LoadQuery ("mysql", "GetEnclosures_query"));

				RemoveMediaRSS_ = QSqlQuery (DB_);
				RemoveMediaRSS_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveMediaRSS_query"));

				RemoveMediaRSSThumbnails_ = QSqlQuery (DB_);
				RemoveMediaRSSThumbnails_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveMediaRSSThumbnails_query"));

				RemoveMediaRSSCredits_ = QSqlQuery (DB_);
				RemoveMediaRSSCredits_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveMediaRSSCredits_query"));

				RemoveMediaRSSComments_ = QSqlQuery (DB_);
				RemoveMediaRSSComments_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveMediaRSSComments_query"));

				RemoveMediaRSSPeerLinks_ = QSqlQuery (DB_);
				RemoveMediaRSSPeerLinks_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveMediaRSSPeerLinks_query"));

				RemoveMediaRSSScenes_ = QSqlQuery (DB_);
				RemoveMediaRSSScenes_.prepare (StorageBackend::LoadQuery ("mysql", "RemoveMediaRSSScenes_query"));
			}

			void SQLStorageBackendMysql::GetFeedsIDs (ids_t& result) const
			{
				QSqlQuery feedSelector (DB_);
				if (!feedSelector.exec ("SELECT feed_id "
							"FROM feeds "
							"ORDER BY feed_id"))
				{
					Util::DBLock::DumpError (feedSelector);
					return;
				}

				while (feedSelector.next ())
					result.push_back (feedSelector.value (0).toInt ());
			}

			Feed_ptr SQLStorageBackendMysql::GetFeed (const IDType_t& feedId) const
			{
				FeedGetter_.bindValue (0, feedId);
				if (!FeedGetter_.exec ())
				{
					Util::DBLock::DumpError (FeedGetter_);
					throw FeedGettingError ();
				}

				if (!FeedGetter_.next ())
				{
					qWarning () << Q_FUNC_INFO
							<< "no feed found with"
							<< feedId;
					throw FeedNotFoundError ();
				}

				Feed_ptr feed (new Feed (feedId));
				feed->URL_ = FeedGetter_.value (0).toString ();
				feed->LastUpdate_ = FeedGetter_.value (1).toDateTime ();
				FeedGetter_.finish ();
				return feed;
			}

			IDType_t SQLStorageBackendMysql::FindFeed (const QString& url) const
			{
				FeedFinderByURL_.bindValue(0,url);
				if (!FeedFinderByURL_.exec ())
				{
					Util::DBLock::DumpError (FeedFinderByURL_);
					throw FeedGettingError ();
				}

				if (!FeedFinderByURL_.next ())
				{
					//TODO check this
					/*
					qWarning () << Q_FUNC_INFO
							<< "no feed for"
							<< url;*/
					return -1;
				}

				IDType_t id = FeedFinderByURL_.value (0).value<IDType_t> ();
				FeedFinderByURL_.finish ();
				return id;
			}

			Feed::FeedSettings SQLStorageBackendMysql::GetFeedSettings (const IDType_t& feedId) const
			{
				FeedSettingsGetter_.bindValue (0, feedId);
				if (!FeedSettingsGetter_.exec ())
				{
					Util::DBLock::DumpError (FeedSettingsGetter_);
					throw std::runtime_error (FeedSettingsGetter_
							.lastError ().text ().toStdString ());
				}

				if (!FeedSettingsGetter_.next ())
					throw FeedSettingsNotFoundError ();

				Feed::FeedSettings result (feedId,
						FeedSettingsGetter_.value (0).value<IDType_t> (),
						FeedSettingsGetter_.value (1).toInt (),
						FeedSettingsGetter_.value (2).toInt (),
						FeedSettingsGetter_.value (3).toInt (),
						FeedSettingsGetter_.value (4).toBool ());
				FeedSettingsGetter_.finish ();

				return result;
			}

			void SQLStorageBackendMysql::SetFeedSettings (const Feed::FeedSettings& settings)
			{
				FeedSettingsSetter_.bindValue (0, settings.FeedID_);		//feed_id
				FeedSettingsSetter_.bindValue (1, settings.SettingsID_);
				FeedSettingsSetter_.bindValue (2, settings.UpdateTimeout_);
				FeedSettingsSetter_.bindValue (3, settings.NumItems_);
				FeedSettingsSetter_.bindValue (4, settings.ItemAge_);
				FeedSettingsSetter_.bindValue (5, settings.AutoDownloadEnclosures_);

				if (!FeedSettingsSetter_.exec ())
					LeechCraft::Util::DBLock::DumpError (FeedSettingsSetter_);
			}

			void SQLStorageBackendMysql::GetChannels (channels_shorts_t& shorts, const IDType_t& feedId) const
			{
				ChannelsShortSelector_.bindValue (0, feedId);				//feed_id
				if (!ChannelsShortSelector_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (ChannelsShortSelector_);
					return;
				}

				while (ChannelsShortSelector_.next ())
				{
					int unread = 0;

					IDType_t id = ChannelsShortSelector_.value (0).value<IDType_t> ();

					UnreadItemsCounter_.bindValue (0, id);					//channel_id
					if (!UnreadItemsCounter_.exec () ||
							!UnreadItemsCounter_.next ())
						Util::DBLock::DumpError (UnreadItemsCounter_);
					else
						unread = UnreadItemsCounter_.value (0).toInt ();

					UnreadItemsCounter_.finish ();

					QStringList tags = Core::Instance ().GetProxy ()->
						GetTagsManager ()->Split (ChannelsShortSelector_.value (3).toString ());
					ChannelShort sh =
					{
						id,
						feedId,
						ChannelsShortSelector_.value (6).toString (),
						ChannelsShortSelector_.value (1).toString (),
						ChannelsShortSelector_.value (2).toString (),
						tags,
						ChannelsShortSelector_.value (4).toDateTime (),
						UnserializePixmap (ChannelsShortSelector_
								.value (5).toByteArray ()),
						unread
					};
					shorts.push_back (sh);
				}

				ChannelsShortSelector_.finish ();
			}

			Channel_ptr SQLStorageBackendMysql::GetChannel (const IDType_t& channelId,
					const IDType_t& parentFeed) const
			{
				ChannelsFullSelector_.bindValue (0, channelId);				//channelID
				if (!ChannelsFullSelector_.exec ())
					Util::DBLock::DumpError (ChannelsFullSelector_);

				if (!ChannelsFullSelector_.next ())
					throw ChannelNotFoundError ();

				Channel_ptr channel (new Channel (parentFeed, channelId));

				channel->Link_ = ChannelsFullSelector_.value (0).toString ();
				channel->Title_ = ChannelsFullSelector_.value (1).toString ();
				channel->Description_ = ChannelsFullSelector_.value (2).toString ();
				channel->LastBuild_ = ChannelsFullSelector_.value (3).toDateTime ();
				QString tags = ChannelsFullSelector_.value (4).toString ();
				channel->Tags_ = Core::Instance ().GetProxy ()->GetTagsManager ()->Split (tags);
				channel->Language_ = ChannelsFullSelector_.value (5).toString ();
				channel->Author_ = ChannelsFullSelector_.value (6).toString ();
				channel->PixmapURL_ = ChannelsFullSelector_.value (7).toString ();
				channel->Pixmap_ = UnserializePixmap (ChannelsFullSelector_
						.value (8).toByteArray ());
				channel->Favicon_ = UnserializePixmap (ChannelsFullSelector_
						.value (9).toByteArray ());

				ChannelsFullSelector_.finish ();

				return channel;
			}

			IDType_t SQLStorageBackendMysql::FindChannel (const QString& title,
					const QString& link, const IDType_t& feedId) const
			{
				ChannelIDFromTitleURL_.bindValue (0, feedId);				//feed_id
				ChannelIDFromTitleURL_.bindValue (1, title);				//title
				ChannelIDFromTitleURL_.bindValue (2, link);					//url
				if (!ChannelIDFromTitleURL_.exec ())
				{
					Util::DBLock::DumpError (ChannelIDFromTitleURL_);
					throw ChannelGettingError ();
				}

				if (!ChannelIDFromTitleURL_.next ())
					throw ChannelNotFoundError ();

				IDType_t result = ChannelIDFromTitleURL_.value (0).value<IDType_t> ();
				ChannelIDFromTitleURL_.finish ();
				return result;
			}

			IDType_t SQLStorageBackendMysql::FindItem (const QString& title,
					const QString& link, const IDType_t& channelId) const
			{
				ItemIDFromTitleURL_.bindValue (0, channelId);				//channel_id
				ItemIDFromTitleURL_.bindValue (1, title);					//title
				ItemIDFromTitleURL_.bindValue (2, link);					//url
				if (!ItemIDFromTitleURL_.exec ())
				{
					Util::DBLock::DumpError (ItemIDFromTitleURL_);
					throw ItemGettingError ();
				}

				if (!ItemIDFromTitleURL_.next ())
					throw ItemNotFoundError ();

				IDType_t result = ItemIDFromTitleURL_.value (0).value<IDType_t> ();
				ItemIDFromTitleURL_.finish ();
				return result;
			}

			void SQLStorageBackendMysql::TrimChannel (const IDType_t& channelId,
					int days, int number)
			{
				ChannelDateTrimmer_.bindValue (0 , channelId);				//channel_id
				ChannelDateTrimmer_.bindValue (1 , days);					//age
				if (!ChannelDateTrimmer_.exec ())
					LeechCraft::Util::DBLock::DumpError (ChannelDateTrimmer_);

				ChannelNumberTrimmer_.bindValue (0, channelId);				//channel_id
				ChannelNumberTrimmer_.bindValue (1, number);				//number
				if (!ChannelNumberTrimmer_.exec ())
					LeechCraft::Util::DBLock::DumpError (ChannelNumberTrimmer_);

				try
				{
					emit channelDataUpdated (GetChannel (channelId,
							FindParentFeedForChannel (channelId)));
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
							<< "channel not found"
							<< channelId;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "error updating channel data"
							<< channelId
							<< e.what ();
				}
			}

			void SQLStorageBackendMysql::GetItems (items_shorts_t& shorts,
					const IDType_t& channelId) const
			{
				ItemsShortSelector_.bindValue (0, channelId);				//channel_id

				if (!ItemsShortSelector_.exec ())
				{
					Util::DBLock::DumpError (ItemsShortSelector_);
					return;
				}

				while (ItemsShortSelector_.next ())
				{
					ItemShort sh =
					{
						ItemsShortSelector_.value (0).value<IDType_t> (),
						channelId,
						ItemsShortSelector_.value (1).toString (),
						ItemsShortSelector_.value (2).toString (),
						ItemsShortSelector_.value (3).toString ()
							.split ("<<<", QString::SkipEmptyParts),
						ItemsShortSelector_.value (4).toDateTime (),
						ItemsShortSelector_.value (5).toBool ()
					};

					shorts.push_back (sh);
				}

				ItemsShortSelector_.finish ();
			}

			int SQLStorageBackendMysql::GetUnreadItems (const IDType_t& channelId) const
			{
				int unread = 0;
				UnreadItemsCounter_.bindValue (0, channelId);				//channel_id
				if (!UnreadItemsCounter_.exec () ||
						!UnreadItemsCounter_.next ())
					Util::DBLock::DumpError (UnreadItemsCounter_);
				else
					unread = UnreadItemsCounter_.value (0).toInt ();

				UnreadItemsCounter_.finish ();
				return unread;
			}

			Item_ptr SQLStorageBackendMysql::GetItem (const IDType_t& itemId) const
			{
				ItemFullSelector_.bindValue (0, itemId);
				if (!ItemFullSelector_.exec ())
					Util::DBLock::DumpError (ItemFullSelector_);

				if (!ItemFullSelector_.next ())
					throw ItemNotFoundError ();

				Item_ptr item (new Item (ItemFullSelector_.value (13).toInt (),
						itemId));
				FillItem (ItemFullSelector_, item);
				ItemFullSelector_.finish ();

				GetEnclosures (itemId, item->Enclosures_);
				GetMRSSEntries (itemId, item->MRSSEntries_);

				return item;
			}

			void SQLStorageBackendMysql::GetItems (items_container_t& items,
					const IDType_t& channelId) const
			{
				ItemsFullSelector_.bindValue (":channel_id", channelId);
				if (!ItemsFullSelector_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (ItemsFullSelector_);
					return;
				}

				while (ItemsFullSelector_.next ())
				{
					IDType_t itemId = ItemsFullSelector_.value (14 ).value<IDType_t> ();
					Item_ptr item (new Item (channelId,
							itemId));
					FillItem (ItemsFullSelector_, item);
					GetEnclosures (itemId, item->Enclosures_);
					GetMRSSEntries (itemId, item->MRSSEntries_);

					items.push_back (item);
				}

				ItemsFullSelector_.finish ();
				GetEnclosures_.finish ();
			}

			void SQLStorageBackendMysql::AddFeed (Feed_ptr feed)
			{
				InsertFeed_.bindValue (0, feed->FeedID_);
				InsertFeed_.bindValue (1, feed->URL_);
				InsertFeed_.bindValue (2, feed->LastUpdate_);
				if (!InsertFeed_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (InsertFeed_);
					return;
				}

				try
				{
					std::for_each (feed->Channels_.begin (), feed->Channels_.end (),
						   boost::bind (&SQLStorageBackendMysql::AddChannel,
							   this,
							   _1));
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}

				InsertFeed_.finish ();
			}

			void SQLStorageBackendMysql::UpdateChannel (Channel_ptr channel)
			{
				ChannelFinder_.bindValue (0, channel->ChannelID_);
				if (!ChannelFinder_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (ChannelFinder_);
					throw std::runtime_error (qPrintable (QString (
									"Unable to execute channel finder query for channel {title: %1; url: %2}")
								.arg (channel->Title_)
								.arg (channel->Link_)));
				}
				if (!ChannelFinder_.next ())
				{
					qWarning () << Q_FUNC_INFO
						<< "not found channel"
						<< channel->Title_
						<< channel->Link_
						<< ", inserting it";
					AddChannel (channel);
					return;
				}
				ChannelFinder_.finish ();

				UpdateChannel_.bindValue (0, channel->ChannelID_);
				UpdateChannel_.bindValue (1, channel->Description_);
				UpdateChannel_.bindValue (2, channel->LastBuild_);
				UpdateChannel_.bindValue (3,
						Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel->Tags_));
				UpdateChannel_.bindValue (4, channel->Language_);
				UpdateChannel_.bindValue (5, channel->Author_);
				UpdateChannel_.bindValue (6, channel->PixmapURL_);
				UpdateChannel_.bindValue (7, SerializePixmap (channel->Pixmap_));
				UpdateChannel_.bindValue (8, SerializePixmap (channel->Favicon_));

				if (!UpdateChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (UpdateChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel t %1, u %2")
								.arg (channel->Title_)
								.arg (channel->Link_)));
				}

				if (!UpdateChannel_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateChannel_";

				UpdateChannel_.finish ();

				emit channelDataUpdated (channel);
			}

			void SQLStorageBackendMysql::UpdateChannel (const ChannelShort& channel)
			{
				ChannelFinder_.bindValue (0, channel.ChannelID_);
				if (!ChannelFinder_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ChannelFinder_);
					throw std::runtime_error (qPrintable (QString (
									"Unable to execute channel finder query {title: %1, url: %2}")
								.arg (channel.Title_)
								.arg (channel.Link_)));
				}
				if (!ChannelFinder_.next ())
				{
					qWarning () << Q_FUNC_INFO;
					throw std::runtime_error (qPrintable (QString (
									"Selected channel for updating doesn't exist and we don't "
									"have enough info to insert it {title: %1, url: %2}")
								.arg (channel.Title_)
								.arg (channel.Link_)));
				}
				ChannelFinder_.finish ();

				UpdateShortChannel_.bindValue (0, channel.ChannelID_);
				UpdateShortChannel_.bindValue (1, channel.LastBuild_);
				UpdateShortChannel_.bindValue (2, Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel.Tags_));

				if (!UpdateShortChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateShortChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel {title: %1, url: %2}")
								.arg (channel.Title_)
								.arg (channel.Link_)));
				}

				if (!UpdateShortChannel_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateShortChannel_";

				UpdateShortChannel_.finish ();

				try
				{
					emit channelDataUpdated (GetChannel (channel.ChannelID_,
							channel.FeedID_));
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< channel.Title_
						<< channel.Link_
						<< channel.ChannelID_;
				}
			}

			void SQLStorageBackendMysql::UpdateItem (Item_ptr item)
			{
				UpdateItem_.bindValue (0, item->ItemID_);
				UpdateItem_.bindValue (1, item->Description_);
				UpdateItem_.bindValue (2, item->Author_);
				UpdateItem_.bindValue (3, item->Categories_.join ("<<<"));
				UpdateItem_.bindValue (4, item->PubDate_);
				UpdateItem_.bindValue (5, item->Unread_);
				UpdateItem_.bindValue (6, item->NumComments_);
				UpdateItem_.bindValue (7, item->CommentsLink_);
				UpdateItem_.bindValue (8, item->CommentsPageLink_);
				UpdateItem_.bindValue (9, QString::number (item->Latitude_));
				UpdateItem_.bindValue (10, QString::number (item->Longitude_));

				if (!UpdateItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (UpdateItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item {id: %1, title: %2, url: %3}")
								.arg (item->ItemID_)
								.arg (item->Title_)
								.arg (item->Link_)));
				}

				if (!UpdateItem_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateItem_";

				UpdateItem_.finish ();

				WriteEnclosures (item->Enclosures_);
				WriteMRSSEntries (item->MRSSEntries_);

				try
				{
					IDType_t cid = item->ChannelID_;
					Channel_ptr channel = GetChannel (cid,
							FindParentFeedForChannel (cid));
					emit itemDataUpdated (item, channel);
					emit channelDataUpdated (channel);
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< item->ChannelID_;
				}
			}

			void SQLStorageBackendMysql::UpdateItem (const ItemShort& item)
			{
				UpdateShortItem_.bindValue (1, item.ItemID_);
				UpdateShortItem_.bindValue (0, item.Unread_);

				if (!UpdateShortItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateShortItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item {id: %1, title: %2, url: %3}")
								.arg (item.ItemID_)
								.arg (item.Title_)
								.arg (item.URL_)));
				}

				/**
				 * @TODO check if we really need this warning
				 *
				 * if (!UpdateShortItem_.numRowsAffected ())
				 *	qWarning () << Q_FUNC_INFO
				 *		<< "no rows affected by UpdateShortItem_";
				 */

				UpdateShortItem_.finish ();

				try
				{
					IDType_t cid = item.ChannelID_;
					Channel_ptr channel = GetChannel (cid,
							FindParentFeedForChannel (cid));
					emit itemDataUpdated (GetItem (item.ItemID_), channel);
					emit channelDataUpdated (channel);
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< item.ChannelID_;
				}
			}

			void SQLStorageBackendMysql::AddChannel (Channel_ptr channel)
			{
				InsertChannel_.bindValue (0, channel->ChannelID_);
				InsertChannel_.bindValue (1, channel->FeedID_);
				InsertChannel_.bindValue (2, channel->Link_);
				InsertChannel_.bindValue (3, channel->Title_);
				InsertChannel_.bindValue (4, channel->Description_);
				InsertChannel_.bindValue (5, channel->LastBuild_);
				InsertChannel_.bindValue (6,
						Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel->Tags_));
				InsertChannel_.bindValue (7, channel->Language_);
				InsertChannel_.bindValue (8, channel->Author_);
				InsertChannel_.bindValue (9, channel->PixmapURL_);
				InsertChannel_.bindValue (10, SerializePixmap (channel->Pixmap_));
				InsertChannel_.bindValue (11, SerializePixmap (channel->Favicon_));

				if (!InsertChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (InsertChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel {id: %1, title: %2, url: %3, parent: %4}")
								.arg (channel->ChannelID_)
								.arg (channel->Title_)
								.arg (channel->Link_)
								.arg (channel->FeedID_)));
				}

				InsertChannel_.finish ();

				std::for_each (channel->Items_.begin (), channel->Items_.end (),
					   boost::bind (&SQLStorageBackendMysql::AddItem,
						   this,
						   _1));
			}

			void SQLStorageBackendMysql::AddItem (Item_ptr item)
			{
				InsertItem_.bindValue (0, item->ItemID_);
				InsertItem_.bindValue (1, item->ChannelID_);
				InsertItem_.bindValue (2, item->Title_);
				InsertItem_.bindValue (3, item->Link_);
				InsertItem_.bindValue (4, item->Description_);
				InsertItem_.bindValue (5, item->Author_);
				InsertItem_.bindValue (6, item->Categories_.join ("<<<"));
				InsertItem_.bindValue (7, item->Guid_);
				InsertItem_.bindValue (8, item->PubDate_);
				InsertItem_.bindValue (9, item->Unread_);
				InsertItem_.bindValue (10, item->NumComments_);
				InsertItem_.bindValue (11, item->CommentsLink_);
				InsertItem_.bindValue (12, item->CommentsPageLink_);
				InsertItem_.bindValue (13, QString::number (item->Latitude_));
				InsertItem_.bindValue (14, QString::number (item->Longitude_));

				if (!InsertItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (InsertItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item {id: %1, channel: %2, title: %3, url: %4")
								.arg (item->ItemID_)
								.arg (item->ChannelID_)
								.arg (item->Title_)
								.arg (item->Link_)));
				}

				InsertItem_.finish ();

				WriteEnclosures (item->Enclosures_);
				WriteMRSSEntries (item->MRSSEntries_);

				try
				{
					IDType_t cid = item->ChannelID_;
					Channel_ptr channel = GetChannel (cid,
							FindParentFeedForChannel (cid));
					emit itemDataUpdated (item, channel);
					emit channelDataUpdated (channel);
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< item->ChannelID_;
				}
			}

			namespace
			{
				bool PerformRemove (QSqlQuery& query,
						const IDType_t& itemId)
				{
					query.bindValue (0, itemId);
					if (!query.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return false;
					}

					query.finish ();

					return true;
				}
			}

			void SQLStorageBackendMysql::RemoveItem (const IDType_t& itemId)
			{
				boost::optional<IDType_t> cid;
				try
				{
					Item_ptr item = GetItem (itemId);
					*cid = item->ChannelID_;
				}
				catch (const ItemNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
							<< "tried to delete item"
							<< itemId
							<< ", but it doesn't exist already";
					return;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to obtain more info on"
							<< itemId
							<< "so won't update channel data";
				}

				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}

				if (!PerformRemove (RemoveEnclosures_, itemId) ||
						!PerformRemove (RemoveMediaRSS_, itemId) ||
						!PerformRemove (RemoveMediaRSSThumbnails_, itemId) ||
						!PerformRemove (RemoveMediaRSSCredits_, itemId) ||
						!PerformRemove (RemoveMediaRSSComments_, itemId) ||
						!PerformRemove (RemoveMediaRSSPeerLinks_, itemId) ||
						!PerformRemove (RemoveMediaRSSScenes_, itemId))
				{
					qWarning () << Q_FUNC_INFO
						<< "a Remove* query failed";
					return;
				}

				RemoveItem_.bindValue (0, itemId);

				if (!RemoveItem_.exec ())
				{
					Util::DBLock::DumpError (RemoveItem_);
					return;
				}

				RemoveItem_.finish ();

				lock.Good ();

				if (cid)
				{
					try
					{
						Channel_ptr channel = GetChannel (*cid,
								FindParentFeedForChannel (*cid));
						emit channelDataUpdated (channel);
					}
					catch (const ChannelNotFoundError&)
					{
						qWarning () << Q_FUNC_INFO
							<< "channel not found"
							<< *cid;
					}
				}
			}

			void SQLStorageBackendMysql::RemoveFeed (const IDType_t& feedId)
			{
				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}

				RemoveFeed_.bindValue (0, feedId);
				if (!RemoveFeed_.exec ())
				{
					Util::DBLock::DumpError (RemoveFeed_);
					return;
				}

				RemoveFeed_.finish ();

				lock.Good ();
			}

			void SQLStorageBackendMysql::ToggleChannelUnread (const IDType_t& channelId,
					bool state)
			{
				items_container_t oldItems;
				GetItems (oldItems, channelId);

				ToggleChannelUnread_.bindValue (0, state);
				ToggleChannelUnread_.bindValue (1, channelId);
				ToggleChannelUnread_.bindValue (2, state);

				if (!ToggleChannelUnread_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ToggleChannelUnread_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to toggle items {cid: %1, state %2}")
								.arg (channelId)
								.arg (state)));
				}

				ToggleChannelUnread_.finish ();

				try
				{
					Channel_ptr channel = GetChannel (channelId,
							FindParentFeedForChannel (channelId));
					emit channelDataUpdated (channel);
					for (size_t i = 0; i < oldItems.size (); ++i)
						if (oldItems.at (i)->Unread_ != state)
						{
							oldItems.at (i)->Unread_ = state;
							emit itemDataUpdated (oldItems.at (i), channel);
						}
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< channelId;
				}
			}

			bool SQLStorageBackendMysql::UpdateFeedsStorage (int, int)
			{
				return true;
			}

			bool SQLStorageBackendMysql::UpdateChannelsStorage (int, int)
			{
				return true;
			}

			bool SQLStorageBackendMysql::UpdateItemsStorage (int oldV, int newV)
			{
				bool success = true;
				/* NOTE No versioning in MySQL yet, so just return true for now.
				while (oldV < newV)
				{
					success = RollItemsStorage (++oldV);
					if (!success)
						break;
				}
				*/

				return success;
			}

			QString SQLStorageBackendMysql::GetBoolType () const
			{
				return "TINYINT";
			}

			QString SQLStorageBackendMysql::GetBlobType () const
			{
				return "BLOB";
			}

			bool SQLStorageBackendMysql::InitializeTables ()
			{
				QSqlQuery query (DB_);
				QStringList names;
				names << "feeds"
						<< "feeds_settings"
						<< "channels"
						<< "items"
						<< "enclosures"
						<< "mrss"
						<< "mrss_thumbnails"
						<< "mrss_credits"
						<< "mrss_comments"
						<< "mrss_scenes";

				Q_FOREACH (const QString& name, names)
				{
					if (!DB_.tables ().contains (name))
						if (!query.exec (StorageBackend::LoadQuery ("mysql",
								QString ("create_table_%1").arg (name))))
						{
							Util::DBLock::DumpError (query);
							return false;
						}
				}

				return true;
			}

			QByteArray SQLStorageBackendMysql::SerializePixmap (const QPixmap& pixmap) const
			{
				QByteArray bytes;
				if (!pixmap.isNull ())
				{
					QBuffer buffer (&bytes);
					buffer.open (QIODevice::WriteOnly);
					pixmap.save (&buffer, "PNG");
				}
				return bytes;
			}

			QPixmap SQLStorageBackendMysql::UnserializePixmap (const QByteArray& bytes) const
			{
				QPixmap result;
				if (bytes.size ())
					result.loadFromData (bytes, "PNG");
				return result;
			}

			void SQLStorageBackendMysql::RemoveTables ()
			{
				struct
				{
					const QSqlDatabase& ThisDB_;

					void operator() (const QString& drstr)
					{
						QSqlQuery dropper = QSqlQuery (ThisDB_);
						if (!dropper.exec (drstr))
						{
							Util::DBLock::DumpError (dropper);
							throw std::runtime_error (qPrintable (dropper
									.lastError ().text ()));
						}
					}
				} rd = { DB_ };
				rd (StorageBackend::LoadQuery ("mysql", "remove_db"));
			}

			IDType_t SQLStorageBackendMysql::FindParentFeedForChannel (const IDType_t& channel) const
			{
				QSqlQuery query (DB_);
				query.prepare ("SELECT feed_id FROM channels WHERE channel_id = ? ");
				query.bindValue (0, channel);
				if (!query.exec ())
				{
					Util::DBLock::DumpError (query);
					throw std::runtime_error ("Unable to find parent feed for channel");
				}

				if (!query.next ())
					throw ChannelNotFoundError ();

				return query.value (0).value<IDType_t> ();
			}

			void SQLStorageBackendMysql::FillItem (const QSqlQuery& query, Item_ptr& item) const
			{
				item->Title_ = query.value (0).toString ();
				item->Link_ = query.value (1).toString ();
				item->Description_ = query.value (2).toString ();
				item->Author_ = query.value (3).toString ();
				item->Categories_ = query.value (4).toString ().split ("<<<", QString::SkipEmptyParts);
				item->Guid_ = query.value (5).toString ();
				item->PubDate_ = query.value (6).toDateTime ();
				item->Unread_ = query.value (7).toBool ();
				item->NumComments_ = query.value (8).toInt ();
				item->CommentsLink_ = query.value (9).toString ();
				item->CommentsPageLink_ = query.value (10).toString ();
				item->Latitude_ = query.value (11).toString ().toDouble ();
				item->Longitude_ = query.value (12).toString ().toDouble ();
				item->ChannelID_ = query.value (13).toString ().toDouble ();
			}

			void SQLStorageBackendMysql::WriteEnclosures (const QList<Enclosure>& enclosures)
			{
				for (QList<Enclosure>::const_iterator i = enclosures.begin (),
						end = enclosures.end (); i != end; ++i)
				{
					WriteEnclosure_.bindValue (4, i->ItemID_);
					WriteEnclosure_.bindValue (5, i->EnclosureID_);
					WriteEnclosure_.bindValue (0, i->URL_);
					WriteEnclosure_.bindValue (1, i->Type_);
					WriteEnclosure_.bindValue (2, i->Length_);
					WriteEnclosure_.bindValue (3, i->Lang_);

					if (!WriteEnclosure_.exec ())
						LeechCraft::Util::DBLock::DumpError (WriteEnclosure_);
				}

				WriteEnclosure_.finish ();
			}

			void SQLStorageBackendMysql::GetEnclosures (const IDType_t& itemId,
					QList<Enclosure>& enclosures) const
			{
				GetEnclosures_.bindValue (0, itemId);

				if (!GetEnclosures_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (GetEnclosures_);
					return;
				}

				while (GetEnclosures_.next ())
				{
					Enclosure e (itemId, GetEnclosures_.value (0).value<IDType_t> ());
					e.URL_ = GetEnclosures_.value (1).toString ();
					e.Type_ = GetEnclosures_.value (2).toString ();
					e.Length_ = GetEnclosures_.value (3).toLongLong ();
					e.Lang_ = GetEnclosures_.value (4).toString ();

					enclosures << e;
				}

				GetEnclosures_.finish ();
			}

			void SQLStorageBackendMysql::WriteMRSSEntries (const QList<MRSSEntry>& entries)
			{
				Q_FOREACH (MRSSEntry e, entries)
				{
					WriteMediaRSS_.bindValue (0, e.MRSSEntryID_);
					WriteMediaRSS_.bindValue (1, e.ItemID_);
					WriteMediaRSS_.bindValue (2, e.URL_);
					WriteMediaRSS_.bindValue (3, e.Size_);
					WriteMediaRSS_.bindValue (4, e.Type_);
					WriteMediaRSS_.bindValue (5, e.Medium_);
					WriteMediaRSS_.bindValue (6, e.IsDefault_);
					WriteMediaRSS_.bindValue (7, e.Expression_);
					WriteMediaRSS_.bindValue (8, e.Bitrate_);
					WriteMediaRSS_.bindValue (9, e.Framerate_);
					WriteMediaRSS_.bindValue (10, e.SamplingRate_);
					WriteMediaRSS_.bindValue (11, e.Channels_);
					WriteMediaRSS_.bindValue (12, e.Duration_);
					WriteMediaRSS_.bindValue (13, e.Width_);
					WriteMediaRSS_.bindValue (14, e.Height_);
					WriteMediaRSS_.bindValue (15, e.Lang_);
					WriteMediaRSS_.bindValue (16, e.Group_);
					WriteMediaRSS_.bindValue (17, e.Rating_);
					WriteMediaRSS_.bindValue (18, e.RatingScheme_);
					WriteMediaRSS_.bindValue (19, e.Title_);
					WriteMediaRSS_.bindValue (20, e.Description_);
					WriteMediaRSS_.bindValue (21, e.Keywords_);
					WriteMediaRSS_.bindValue (22, e.CopyrightURL_);
					WriteMediaRSS_.bindValue (23, e.CopyrightText_);
					WriteMediaRSS_.bindValue (24, e.RatingAverage_);
					WriteMediaRSS_.bindValue (25, e.RatingCount_);
					WriteMediaRSS_.bindValue (26, e.RatingMin_);
					WriteMediaRSS_.bindValue (27, e.RatingMax_);
					WriteMediaRSS_.bindValue (28, e.Views_);
					WriteMediaRSS_.bindValue (29, e.Favs_);
					WriteMediaRSS_.bindValue (30, e.Tags_);

					if (!WriteMediaRSS_.exec ())
					{
						Util::DBLock::DumpError (WriteMediaRSS_);
						continue;
					}

					WriteMediaRSS_.finish ();

					Q_FOREACH (MRSSThumbnail t, e.Thumbnails_)
					{
						WriteMediaRSSThumbnail_.bindValue (0, t.MRSSThumbnailID_);
						WriteMediaRSSThumbnail_.bindValue (1, t.MRSSEntryID_);
						WriteMediaRSSThumbnail_.bindValue (2, t.URL_);
						WriteMediaRSSThumbnail_.bindValue (3, t.Width_);
						WriteMediaRSSThumbnail_.bindValue (4, t.Height_);
						WriteMediaRSSThumbnail_.bindValue (5, t.Time_);

						if (!WriteMediaRSSThumbnail_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSThumbnail_);

						WriteMediaRSSThumbnail_.finish ();
					}

					Q_FOREACH (MRSSCredit c, e.Credits_)
					{
						WriteMediaRSSCredit_.bindValue (0, c.MRSSCreditID_);
						WriteMediaRSSCredit_.bindValue (1, c.MRSSEntryID_);
						WriteMediaRSSCredit_.bindValue (2, c.Role_);
						WriteMediaRSSCredit_.bindValue (3, c.Who_);

						if (!WriteMediaRSSCredit_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSCredit_);

						WriteMediaRSSCredit_.finish ();
					}

					Q_FOREACH (MRSSComment c, e.Comments_)
					{
						WriteMediaRSSComment_.bindValue (0, c.MRSSCommentID_);
						WriteMediaRSSComment_.bindValue (1, c.MRSSEntryID_);
						WriteMediaRSSComment_.bindValue (2, c.Type_);
						WriteMediaRSSComment_.bindValue (3, c.Comment_);

						if (!WriteMediaRSSComment_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSComment_);

						WriteMediaRSSComment_.finish ();
					}

					Q_FOREACH (MRSSPeerLink p, e.PeerLinks_)
					{
						WriteMediaRSSPeerLink_.bindValue (0, p.MRSSPeerLinkID_);
						WriteMediaRSSPeerLink_.bindValue (1, p.MRSSEntryID_);
						WriteMediaRSSPeerLink_.bindValue (2, p.Type_);
						WriteMediaRSSPeerLink_.bindValue (3, p.Link_);

						if (!WriteMediaRSSPeerLink_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSPeerLink_);

						WriteMediaRSSPeerLink_.finish ();
					}

					Q_FOREACH (MRSSScene s, e.Scenes_)
					{
						WriteMediaRSSScene_.bindValue (0, s.MRSSSceneID_);
						WriteMediaRSSScene_.bindValue (1, s.MRSSEntryID_);
						WriteMediaRSSScene_.bindValue (2, s.Title_);
						WriteMediaRSSScene_.bindValue (3, s.Description_);
						WriteMediaRSSScene_.bindValue (4, s.StartTime_);
						WriteMediaRSSScene_.bindValue (5, s.EndTime_);

						if (!WriteMediaRSSScene_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSScene_);

						WriteMediaRSSScene_.finish ();
					}
				}
			}

			void SQLStorageBackendMysql::GetMRSSEntries (const IDType_t& itemId, QList<MRSSEntry>& entries) const
			{
				GetMediaRSSs_.bindValue (0, itemId);

				if (!GetMediaRSSs_.exec ())
				{
					Util::DBLock::DumpError (GetMediaRSSs_);
					return;
				}

				while (GetMediaRSSs_.next ())
				{
					IDType_t mrssId = GetMediaRSSs_.value (0).value<IDType_t> ();
					MRSSEntry e (itemId, mrssId);
					e.URL_ = GetMediaRSSs_.value (1).toString ();
					e.Size_ = GetMediaRSSs_.value (2).toLongLong ();
					e.Type_ = GetMediaRSSs_.value (3).toString ();
					e.Medium_ = GetMediaRSSs_.value (4).toString ();
					e.IsDefault_ = GetMediaRSSs_.value (5).toBool ();
					e.Expression_ = GetMediaRSSs_.value (6).toString ();
					e.Bitrate_ = GetMediaRSSs_.value (7).toInt ();
					e.Framerate_ = GetMediaRSSs_.value (8).toDouble ();
					e.SamplingRate_ = GetMediaRSSs_.value (9).toDouble ();
					e.Channels_ = GetMediaRSSs_.value (10).toInt ();
					e.Duration_ = GetMediaRSSs_.value (11).toInt ();
					e.Width_ = GetMediaRSSs_.value (12).toInt ();
					e.Height_ = GetMediaRSSs_.value (13).toInt ();
					e.Lang_ = GetMediaRSSs_.value (14).toString ();
					e.Group_ = GetMediaRSSs_.value (15).toInt ();
					e.Rating_ = GetMediaRSSs_.value (16).toString ();
					e.RatingScheme_ = GetMediaRSSs_.value (17).toString ();
					e.Title_ = GetMediaRSSs_.value (18).toString ();
					e.Description_ = GetMediaRSSs_.value (19).toString ();
					e.Keywords_ = GetMediaRSSs_.value (20).toString ();
					e.CopyrightURL_ = GetMediaRSSs_.value (21).toString ();
					e.CopyrightText_ = GetMediaRSSs_.value (22).toString ();
					e.RatingAverage_ = GetMediaRSSs_.value (23).toInt ();
					e.RatingCount_ = GetMediaRSSs_.value (24).toInt ();
					e.RatingMin_ = GetMediaRSSs_.value (25).toInt ();
					e.RatingMax_ = GetMediaRSSs_.value (26).toInt ();
					e.Views_ = GetMediaRSSs_.value (27).toInt ();
					e.Favs_ = GetMediaRSSs_.value (28).toInt ();
					e.Tags_ = GetMediaRSSs_.value (29).toString ();

					GetMediaRSSThumbnails_.bindValue (0, mrssId);
					if (!GetMediaRSSThumbnails_.exec ())
						Util::DBLock::DumpError (GetMediaRSSThumbnails_);
					else
					{
						while (GetMediaRSSThumbnails_.next ())
						{
							MRSSThumbnail th (mrssId,
									GetMediaRSSThumbnails_.value (0).value<IDType_t> ());
							th.URL_ = GetMediaRSSThumbnails_.value (1).toString ();
							th.Width_ = GetMediaRSSThumbnails_.value (2).toInt ();
							th.Height_ = GetMediaRSSThumbnails_.value (3).toInt ();
							th.Time_ = GetMediaRSSThumbnails_.value (4).toString ();
							e.Thumbnails_ << th;
						}
						GetMediaRSSThumbnails_.finish ();
					}

					GetMediaRSSCredits_.bindValue (0, mrssId);
					if (!GetMediaRSSCredits_.exec ())
						Util::DBLock::DumpError (GetMediaRSSCredits_);
					else
					{
						while (GetMediaRSSCredits_.next ())
						{
							MRSSCredit cr (mrssId,
									GetMediaRSSCredits_.value (0).value<IDType_t> ());
							cr.Role_ = GetMediaRSSCredits_.value (1).toString ();
							cr.Who_ = GetMediaRSSCredits_.value (2).toString ();
							e.Credits_ << cr;
						}
						GetMediaRSSCredits_.finish ();
					}

					GetMediaRSSComments_.bindValue (0, mrssId);
					if (!GetMediaRSSComments_.exec ())
						Util::DBLock::DumpError (GetMediaRSSComments_);
					else
					{
						while (GetMediaRSSComments_.next ())
						{
							MRSSComment cm (mrssId,
									GetMediaRSSComments_.value (0).value<IDType_t> ());
							cm.Type_ = GetMediaRSSComments_.value (1).toString ();
							cm.Comment_ = GetMediaRSSComments_.value (2).toString ();
							e.Comments_ << cm;
						}
						GetMediaRSSComments_.finish ();
					}

					GetMediaRSSPeerLinks_.bindValue (0, mrssId);
					if (!GetMediaRSSPeerLinks_.exec ())
						Util::DBLock::DumpError (GetMediaRSSPeerLinks_);
					else
					{
						while (GetMediaRSSPeerLinks_.next ())
						{
							MRSSPeerLink pl (mrssId,
									GetMediaRSSPeerLinks_.value (0).value<IDType_t> ());
							pl.Type_ = GetMediaRSSPeerLinks_.value (1).toString ();
							pl.Link_ = GetMediaRSSPeerLinks_.value (2).toString ();
							e.PeerLinks_ << pl;
						}
						GetMediaRSSPeerLinks_.finish ();
					}

					GetMediaRSSScenes_.bindValue (0, mrssId);
					if (!GetMediaRSSScenes_.exec ())
						Util::DBLock::DumpError (GetMediaRSSScenes_);
					else
					{
						while (GetMediaRSSScenes_.next ())
						{
							MRSSScene th (mrssId,
									GetMediaRSSScenes_.value (0).value<IDType_t> ());
							th.Title_ = GetMediaRSSScenes_.value (1).toString ();
							th.Description_ = GetMediaRSSScenes_.value (2).toString ();
							th.StartTime_ = GetMediaRSSScenes_.value (3).toString ();
							th.EndTime_ = GetMediaRSSScenes_.value (4).toString ();
							e.Scenes_ << th;
						}
						GetMediaRSSScenes_.finish ();
					}

					entries << e;
				}

				GetMediaRSSs_.finish ();
			}
		};
	};
};
