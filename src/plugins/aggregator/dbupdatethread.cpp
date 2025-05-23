/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dbupdatethread.h"
#include <QtConcurrent>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include "components/storage/storagebackendmanager.h"
#include "common.h"
#include "dbutils.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	DBUpdateThread::DBUpdateThread (QObject *parent)
	: QObject { parent }
	{
		Pool_.setMaxThreadCount (1);
	}

	QFuture<void> DBUpdateThread::SetAllChannelsRead ()
	{
		return QtConcurrent::run (&Pool_,
				[]
				{
					const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
					for (const auto& channel : GetAllChannels ())
						sb->ToggleChannelUnread (channel.ChannelID_, false);
				});
	}

	QFuture<void> DBUpdateThread::ToggleChannelUnread (IDType_t channelId, bool unread)
	{
		return QtConcurrent::run (&Pool_,
				[=]
				{
					const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
					sb->ToggleChannelUnread (channelId, unread);
				});
	}

	namespace
	{
		class UpdateFeedWorker
		{
			const StorageBackend_ptr SB_ = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

			IDType_t FeedID_;
			Feed::FeedSettings FeedSettings_;
		public:
			UpdateFeedWorker (channels_container_t& channels, const QString& url)
			{
				const auto maybeFeedId = SB_->FindFeed (url);
				if (!maybeFeedId)
				{
					qWarning () << Q_FUNC_INFO
						<< "skipping"
						<< url
						<< "cause seems like it's not in storage yet";
					return;
				}
				FeedID_ = *maybeFeedId;
				LoadFeedSettings ();

				for (auto& channel : channels)
				{
					FixItems (*channel);
					TrimChannel (*channel);

					if (const auto& maybeOurChannelID = FindMatchingChannel (*channel, channels.size () == 1))
						UpdateChannel (*maybeOurChannelID, *channel);
					else
						AddChannel (*channel);
				}
			}
		private:
			void LoadFeedSettings ()
			{
				FeedSettings_.ItemAge_ = XmlSettingsManager::Instance ().property ("ItemsMaxAge").toInt ();
				FeedSettings_.NumItems_ = XmlSettingsManager::Instance ().property ("ItemsPerChannel").toUInt ();

				const auto& settings = SB_->GetFeedSettings (FeedID_);
				if (!settings)
					return;

				if (settings->ItemAge_)
					FeedSettings_.ItemAge_ = settings->ItemAge_;
				if (settings->NumItems_)
					FeedSettings_.NumItems_ = settings->NumItems_;
			}

			std::optional<IDType_t> FindMatchingChannel (const Channel& channel, bool withFuzzy) const
			{
				if (const auto directMatch = SB_->FindChannel (channel.Title_, channel.Link_, FeedID_))
					return directMatch;

				if (!withFuzzy)
					return {};

				const auto& allLocalChannels = SB_->GetChannels (FeedID_);
				if (allLocalChannels.size () != 1)
					return {};

				auto localChannel = allLocalChannels.at (0);

				qDebug () << "correcting local channel with params"
						<< localChannel.Title_
						<< localChannel.Link_;

				if (localChannel.Title_ != channel.Title_)
					SB_->SetChannelTitle (localChannel.ChannelID_, channel.Title_);
				if (localChannel.Link_ != channel.Link_)
					SB_->SetChannelLink (localChannel.ChannelID_, channel.Link_);

				return localChannel.ChannelID_;
			}

			void FixItems (Channel& channel)
			{
				for (const auto& item : channel.Items_)
					item->FixDate ();
			}

			void TrimChannel (Channel& channel)
			{
				const auto& now = QDateTime::currentDateTime ();
				const auto isOld = [&] (auto&& item) { return item->PubDate_.daysTo (now) >= FeedSettings_.ItemAge_; };
				const auto erasePos = std::remove_if (channel.Items_.begin (), channel.Items_.end (), isOld);
				channel.Items_.erase (erasePos, channel.Items_.end ());

				if (channel.Items_.size () > FeedSettings_.NumItems_)
				{
					// sort by publication date in reverse order
					std::stable_sort (channel.Items_.begin (), channel.Items_.end (),
							[] (const Item_ptr& left, const Item_ptr& right) { return left->PubDate_ > right->PubDate_; });
					channel.Items_.resize (FeedSettings_.NumItems_);
				}
			}

			void AddChannel (Channel& channel) const
			{
				if (const auto tags = SB_->GetFeedTags (channel.FeedID_))
				{
					channel.Tags_ += *tags;
					channel.Tags_.removeDuplicates ();
				}

				SB_->AddChannel (channel);

				auto str = DBUpdateThread::tr ("Added channel \"%1\" (%n item(s))", nullptr, channel.Items_.size ())
						.arg (channel.Title_);
				GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification (NotificationTitle, str, Priority::Info));
			}

			void UpdateChannel (IDType_t ourChannelId, Channel& channel) const
			{
				const auto& ourChannel = SB_->GetChannel (ourChannelId);

				int newItems = 0;
				int updatedItems = 0;

				for (const auto& itemPtr : channel.Items_)
				{
					auto& item = *itemPtr;
					if (const auto& ourItemId = FindOurItem (item, ourChannel.ChannelID_);
						const auto& ourItem = SB_->GetItem (*ourItemId))
					{
						if (UpdateItem (item, *ourItem))
							++updatedItems;
					}
					else
					{
						item.ChannelID_ = ourChannel.ChannelID_;
						AddItem (item, ourChannel);
						++newItems;
					}
				}

				SB_->TrimChannel (ourChannelId, FeedSettings_.ItemAge_, FeedSettings_.NumItems_);

				NotifyUpdates (newItems, updatedItems, channel);
			}

			std::optional<IDType_t> FindOurItem (const Item& item, IDType_t channelId) const
			{
				if (const auto id = SB_->FindItem (item.Title_, item.Link_, channelId))
					return id;
				if (const auto id = SB_->FindItemByLink (item.Link_, channelId))
					return id;

				if (!item.Link_.isEmpty ())
					return {};

				return SB_->FindItemByTitle (item.Title_, channelId);
			}

			void AddItem (Item& item, const Channel& channel) const
			{
				SB_->AddItem (item);

				if (FeedSettings_.AutoDownloadEnclosures_)
					DownloadEnclosures (item.Enclosures_, channel.Tags_);
			}

			void DownloadEnclosures (const QList<Enclosure>& enclosures, const QStringList& tags) const
			{
				const auto iem = GetProxyHolder ()->GetEntityManager ();
				const auto& targetPath = XmlSettingsManager::Instance ().property ("EnclosuresDownloadPath").toString ();
				for (const auto& enc : enclosures)
				{
					auto de = Util::MakeEntity (QUrl { enc.URL_ }, targetPath, {}, enc.Type_);
					de.Additional_ [" Tags"] = tags;
					iem->HandleEntity (de);
				}
			}

			bool UpdateItem (const Item& item, Item ourItem) const
			{
				if (!IsModified (ourItem, item))
					return false;

				static const bool debugDiffs = qgetenv ("LC_AGGREGATOR_DUMP_DIFFS") == "1";
				if (debugDiffs)
					Diff (ourItem, item);

				ourItem.Description_ = item.Description_;
				ourItem.Categories_ = item.Categories_;
				ourItem.NumComments_ = item.NumComments_;
				ourItem.CommentsLink_ = item.CommentsLink_;
				ourItem.CommentsPageLink_ = item.CommentsPageLink_;
				ourItem.Latitude_ = item.Latitude_;
				ourItem.Longitude_ = item.Longitude_;

				for (auto enc : item.Enclosures_)
					if (!ourItem.Enclosures_.contains (enc))
					{
						enc.ItemID_ = ourItem.ItemID_;
						ourItem.Enclosures_ << enc;
					}

				for (auto entry : item.MRSSEntries_)
					if (!ourItem.MRSSEntries_.contains (entry))
					{
						entry.ItemID_ = ourItem.ItemID_;
						ourItem.MRSSEntries_ << entry;
					}

				SB_->UpdateItem (ourItem);

				return true;
			}

			void NotifyUpdates (int newItems, int updatedItems, const Channel& channel) const
			{
				const auto& method = XmlSettingsManager::Instance ().property ("NotificationsFeedUpdateBehavior").toString ();
				if (method == "ShowNo")
					return;
				if (method == "ShowNew" && !newItems)
					return;
				if (method == "ShowAll" && !newItems && !updatedItems)
					return;

				QStringList substrs;
				if (newItems)
					substrs << DBUpdateThread::tr ("%n new item(s)", "Channel update", newItems);
				if (updatedItems)
					substrs << DBUpdateThread::tr ("%n updated item(s)", "Channel update", updatedItems);
				const auto& str = DBUpdateThread::tr ("Updated channel \"%1\" (%2).")
						.arg (channel.Title_)
						.arg (substrs.join (", "));
				GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification (NotificationTitle, str, Priority::Info));
			}
		};
	}

	QFuture<void> DBUpdateThread::UpdateFeed (channels_container_t channels, QString url)
	{
		return QtConcurrent::run (&Pool_,
				[channels = std::move (channels), url = std::move (url)] () mutable
				{
					UpdateFeedWorker { channels, url };
				});
	}
}
