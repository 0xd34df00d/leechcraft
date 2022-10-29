/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dbupdatethreadworker.h"
#include <stdexcept>
#include <QUrl>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/xpc/defaulthookproxy.h>
#include <util/sll/lazy.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"
#include "storagebackend.h"
#include "storagebackendmanager.h"

namespace LC
{
namespace Aggregator
{
	DBUpdateThreadWorker::DBUpdateThreadWorker (QObject *parent)
	: QObject (parent)
	, SB_ { StorageBackendManager::Instance ().MakeStorageBackendForThread () }
	{
		SB_->Prepare ();
	}

	void DBUpdateThreadWorker::WithWorker (const std::function<void (DBUpdateThreadWorker*)>& func)
	{
		func (this);
	}

	Feed::FeedSettings DBUpdateThreadWorker::GetFeedSettings (IDType_t feedId)
	{
		const auto itemAge = XmlSettingsManager::Instance ()->property ("ItemsMaxAge").toInt ();
		const auto items = XmlSettingsManager::Instance ()->property ("ItemsPerChannel").toUInt ();

		if (const auto& maybeSettings = SB_->GetFeedSettings (feedId))
		{
			auto settings = *maybeSettings;
			if (!settings.ItemAge_)
				settings.ItemAge_ = itemAge;
			if (!settings.NumItems_)
				settings.NumItems_ = items;
			return settings;
		}

		Feed::FeedSettings s { feedId };
		s.ItemAge_ = itemAge;
		s.NumItems_ = items;
		s.AutoDownloadEnclosures_ = false;
		return s;
	}

	void DBUpdateThreadWorker::AddChannel (Channel channel)
	{
		if (const auto tags = SB_->GetFeedTags (channel.FeedID_))
		{
			channel.Tags_ += *tags;
			channel.Tags_.removeDuplicates ();
		}

		SB_->AddChannel (channel);

		QString str = tr ("Added channel \"%1\" (%n item(s))",
				"", channel.Items_.size ())
			.arg (channel.Title_);

		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Aggregator", str, Priority::Info));
	}

	bool DBUpdateThreadWorker::AddItem (Item& item, const Channel& channel, const Feed::FeedSettings& settings)
	{
		if (item.PubDate_.isValid ())
		{
			if (item.PubDate_.daysTo (QDateTime::currentDateTime ()) >= settings.ItemAge_)
				return false;
		}
		else
			item.FixDate ();

		item.ChannelID_ = channel.ChannelID_;
		SB_->AddItem (item);

		const auto iem = GetProxyHolder ()->GetEntityManager ();
		if (settings.AutoDownloadEnclosures_)
			for (const auto& e : item.Enclosures_)
			{
				auto de = Util::MakeEntity (QUrl (e.URL_),
						XmlSettingsManager::Instance ()->property ("EnclosuresDownloadPath").toString (),
						{},
						e.Type_);
				de.Additional_ [" Tags"] = channel.Tags_;
				iem->HandleEntity (de);
			}

		return true;
	}

	bool DBUpdateThreadWorker::UpdateItem (const Item& item, Item ourItem)
	{
		if (!IsModified (ourItem, item))
			return false;

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

	void DBUpdateThreadWorker::NotifyUpdates (int newItems, int updatedItems, const Channel_ptr& channel)
	{
		const auto& method = XmlSettingsManager::Instance ()->
				property ("NotificationsFeedUpdateBehavior").toString ();
		bool shouldShow = true;
		if (method == "ShowNo")
			shouldShow = false;
		else if (method == "ShowNew")
			shouldShow = newItems;
		else if (method == "ShowAll")
			shouldShow = newItems + updatedItems;

		if (!shouldShow)
			return;

		QStringList substrs;
		if (newItems)
			substrs << tr ("%n new item(s)", "Channel update", newItems);
		if (updatedItems)
			substrs << tr ("%n updated item(s)", "Channel update", updatedItems);
		const auto& str = tr ("Updated channel \"%1\" (%2).")
				.arg (channel->Title_)
				.arg (substrs.join (", "));
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Aggregator", str, Priority::Info));
	}

	std::optional<IDType_t> DBUpdateThreadWorker::MatchChannel (const Channel& channel, IDType_t feedId,
			const channels_container_t& allRemoteChannels) const
	{
		if (const auto directMatch = SB_->FindChannel (channel.Title_, channel.Link_, feedId))
			return directMatch;

		qDebug () << Q_FUNC_INFO
				<< "unable to find a channel directly matching"
				<< channel.Title_
				<< channel.Link_
				<< feedId;

		const auto& allLocalChannels = SB_->GetChannels (feedId);
		if (allLocalChannels.size () != 1 || allRemoteChannels.size () != 1)
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

	void DBUpdateThreadWorker::toggleChannelUnread (IDType_t channel, bool state)
	{
		SB_->ToggleChannelUnread (channel, state);
	}

	void DBUpdateThreadWorker::updateFeed (channels_container_t channels, QString url)
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
		const auto feedId = *maybeFeedId;

		const auto& feedSettings = GetFeedSettings (feedId);
		const auto ipc = feedSettings.NumItems_;
		const auto days = feedSettings.ItemAge_;

		for (const auto& channel : channels)
		{
			const auto maybeOurChannelID = MatchChannel (*channel, feedId, channels);
			if (!maybeOurChannelID)
			{
				AddChannel (*channel);
				continue;
			}

			const auto ourChannelID = *maybeOurChannelID;
			const auto& ourChannel = SB_->GetChannel (ourChannelID);

			int newItems = 0;
			int updatedItems = 0;

			for (const auto& itemPtr : channel->Items_)
			{
				auto& item = *itemPtr;

				auto mkLazy = [] (auto&& f) { return Util::MakeLazyF<std::optional<IDType_t>> (f); };
				const auto& ourItemID = Util::Msum ({
							mkLazy ([&] { return SB_->FindItem (item.Title_, item.Link_, ourChannel.ChannelID_); }),
							mkLazy ([&] { return SB_->FindItemByLink (item.Link_, ourChannel.ChannelID_); }),
							mkLazy ([&]
									{
										if (!item.Link_.isEmpty ())
											return std::optional<IDType_t> {};

										return SB_->FindItemByTitle (item.Title_, ourChannel.ChannelID_);
									})
						}) ();
				if (ourItemID)
				{
					if (const auto& ourItem = SB_->GetItem (*ourItemID))
						if (UpdateItem (item, *ourItem))
							++updatedItems;
				}
				else if (AddItem (item, ourChannel, feedSettings))
					++newItems;
			}

			SB_->TrimChannel (ourChannel.ChannelID_, days, ipc);

			NotifyUpdates (newItems, updatedItems, channel);
		}
	}
}
}
