/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "dbupdatethreadworker.h"
#include <stdexcept>
#include <QUrl>
#include <QtDebug>
#include <util/util.h>
#include <util/defaulthookproxy.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "storagebackend.h"
#include "regexpmatchermanager.h"
#include "tovarmaps.h"

namespace LeechCraft
{
namespace Aggregator
{
	DBUpdateThreadWorker::DBUpdateThreadWorker (QObject *parent)
	: QObject (parent)
	{
		try
		{
			const QString& strType = XmlSettingsManager::Instance ()->
					property ("StorageType").toString ();
			SB_ = StorageBackend::Create (strType, "_UpdateThread");
		}
		catch (const std::runtime_error& s)
		{
			qWarning () << Q_FUNC_INFO
					<< s.what ();
			return;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown exception";
			return;
		}

		SB_->Prepare ();

		connect (SB_.get (),
				SIGNAL (channelDataUpdated (Channel_ptr)),
				this,
				SLOT (handleChannelDataUpdated (Channel_ptr)));
		connect (SB_.get (),
				SIGNAL (itemDataUpdated (Item_ptr, Channel_ptr)),
				this,
				SIGNAL (itemDataUpdated (Item_ptr, Channel_ptr)));
	}

	void DBUpdateThreadWorker::toggleChannelUnread (IDType_t channel, bool state)
	{
		SB_->ToggleChannelUnread (channel, state);
	}

	void DBUpdateThreadWorker::updateFeed (channels_container_t channels, QString url)
	{
		IDType_t feedId = SB_->FindFeed (url);
		if (feedId == static_cast<IDType_t> (-1))
		{
			qWarning () << Q_FUNC_INFO
				<< "skipping"
				<< url
				<< "cause seems like it's not in storage yet";
			return;
		}

		const int defaultDays = XmlSettingsManager::Instance ()->
			property ("ItemsMaxAge").toInt ();
		const unsigned defaultIpc = XmlSettingsManager::Instance ()->
			property ("ItemsPerChannel").value<unsigned> ();
		bool downloadEnclosures = false;

		int days = defaultDays;
		unsigned ipc = defaultIpc;
		try
		{
			const auto& settings = SB_->GetFeedSettings (feedId);
			if (settings.ItemAge_)
				days = settings.ItemAge_;
			if (settings.NumItems_)
				ipc = settings.NumItems_;
			downloadEnclosures = settings.AutoDownloadEnclosures_;
		}
		catch (const StorageBackend::FeedSettingsNotFoundError&)
		{
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get feed settings for"
					<< feedId
					<< url
					<< e.what ();
		}

		QDateTime current = QDateTime::currentDateTime ();
		Q_FOREACH (Channel_ptr channel, channels)
		{
			Channel_ptr ourChannel;
			try
			{
				IDType_t ourChannelID = SB_->FindChannel (channel->Title_,
						channel->Link_, feedId);
				ourChannel = SB_->GetChannel (ourChannelID, feedId);
			}
			catch (const StorageBackend::ChannelNotFoundError&)
			{
				size_t truncateAt = (channel->Items_.size () <= ipc) ?
					channel->Items_.size () : ipc;
				for (size_t j = 0; j < channel->Items_.size (); j++)
					if (channel->Items_ [j]->PubDate_.daysTo (current) > days)
					{
						truncateAt = std::min (j, truncateAt);
						break;
					}
				channel->Items_.resize (truncateAt);

				emit gotNewChannel (channel->ToShort ());
				SB_->AddChannel (channel);
				QString str = tr ("Added channel \"%1\" (%n item(s))",
						"", channel->Items_.size ())
					.arg (channel->Title_);
				emit gotEntity (Util::MakeNotification ("Aggregator", str, PInfo_));
				continue;
			}

			const QVariantMap& channelPart = GetItemMapChannelPart (ourChannel);

			int newItems = 0;
			int updatedItems = 0;

			Core::Instance ().GetPool (PTChannel).FreeID (channel->ChannelID_);

			Q_FOREACH (Item_ptr item, channel->Items_)
			{
				Item_ptr ourItem;
				try
				{
					IDType_t ourItemID = SB_->FindItem (item->Title_, item->Link_,
							ourChannel->ChannelID_);
					ourItem = SB_->GetItem (ourItemID);
				}
				catch (const StorageBackend::ItemNotFoundError&)
				{
					if (item->PubDate_.isValid ())
					{
						if (item->PubDate_.daysTo (current) >= days)
							continue;
					}
					else
						item->FixDate ();

					item->ChannelID_ = ourChannel->ChannelID_;
					SB_->AddItem (item);

					RegexpMatcherManager::Instance ().HandleItem (item);

					QVariantList itemData;
					itemData << GetItemMapItemPart (item).unite (channelPart);
					emit hookGotNewItems (Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy),
							itemData);

					if (downloadEnclosures)
						Q_FOREACH (Enclosure e, item->Enclosures_)
						{
							Entity de = Util::MakeEntity (QUrl (e.URL_),
									XmlSettingsManager::Instance ()->
										property ("EnclosuresDownloadPath").toString (),
									0,
									e.Type_);
							de.Additional_ [" Tags"] = channel->Tags_;
							emit gotEntity (de);
						}
					++newItems;
					continue;
				}

				if (!IsModified (ourItem, item))
					continue;

				ourItem->Description_ = item->Description_;
				ourItem->Categories_ = item->Categories_;
				ourItem->NumComments_ = item->NumComments_;
				ourItem->CommentsLink_ = item->CommentsLink_;
				ourItem->CommentsPageLink_ = item->CommentsPageLink_;
				ourItem->Latitude_ = item->Latitude_;
				ourItem->Longitude_ = item->Longitude_;

				Q_FOREACH (Enclosure enc, item->Enclosures_)
				{
					if (ourItem->Enclosures_.contains (enc))
						Core::Instance ().GetPool (PTEnclosure).FreeID (enc.EnclosureID_);
					else
					{
						enc.ItemID_ = ourItem->ItemID_;
						ourItem->Enclosures_ << enc;
					}
				}

				Q_FOREACH (MRSSEntry entry, item->MRSSEntries_)
				{
					if (ourItem->MRSSEntries_.contains (entry))
					{
						Core::Instance ().GetPool (PTMRSSEntry).FreeID (entry.MRSSEntryID_);

						Q_FOREACH (MRSSComment comment, entry.Comments_)
							Core::Instance ().GetPool (PTMRSSComment).FreeID (comment.MRSSCommentID_);
						Q_FOREACH (MRSSCredit credit, entry.Credits_)
							Core::Instance ().GetPool (PTMRSSCredit).FreeID (credit.MRSSCreditID_);
						Q_FOREACH (MRSSPeerLink peerLink, entry.PeerLinks_)
							Core::Instance ().GetPool (PTMRSSPeerLink).FreeID (peerLink.MRSSPeerLinkID_);
						Q_FOREACH (MRSSThumbnail thumb, entry.Thumbnails_)
							Core::Instance ().GetPool (PTMRSSThumbnail).FreeID (thumb.MRSSThumbnailID_);
						Q_FOREACH (MRSSScene scene, entry.Scenes_)
							Core::Instance ().GetPool (PTMRSSScene).FreeID (scene.MRSSSceneID_);
					}
					else
					{
						entry.ItemID_ = ourItem->ItemID_;
						ourItem->MRSSEntries_ << entry;
					}
				}

				Core::Instance ().GetPool (PTItem).FreeID (item->ItemID_);

				SB_->UpdateItem (ourItem);
				++updatedItems;
			}

			QString method = XmlSettingsManager::Instance ()->
					property ("NotificationsFeedUpdateBehavior").toString ();
			bool shouldShow = true;
			if (method == "ShowNo")
				shouldShow = false;
			else if (method == "ShowNew")
				shouldShow = newItems;
			else if (method == "ShowAll")
				shouldShow = newItems + updatedItems;

			if (shouldShow)
			{
				QString str = tr ("Updated channel \"%1\" (%2, %3)").arg (channel->Title_)
					.arg (tr ("%n new item(s)", "Channel update", newItems))
					.arg (tr ("%n updated item(s)", "Channel update", updatedItems));
				emit gotEntity (Util::MakeNotification ("Aggregator", str, PInfo_));
			}

			SB_->TrimChannel (ourChannel->ChannelID_,
					days, ipc);
		}
	}

	void DBUpdateThreadWorker::handleChannelDataUpdated (Channel_ptr ch)
	{
		emit channelDataUpdated (ch->ChannelID_, ch->FeedID_);
	}
}
}
