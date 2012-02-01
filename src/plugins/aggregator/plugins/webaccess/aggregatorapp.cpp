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

#include "aggregatorapp.h"
#include <QObject>
#include <QtDebug>
#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WBoxLayout>
#include <Wt/WCheckBox>
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WOverlayLoadingIndicator>
#include <util/util.h>
#include <interfaces/aggregator/iproxyobject.h>
#include <interfaces/aggregator/channel.h>
#include "readchannelsfilter.h"

namespace LeechCraft
{
namespace Aggregator
{
namespace WebAccess
{
	namespace
	{
		Wt::WString ToW (const QString& str)
		{
			return Wt::WString (str.toUtf8 ().constData (), Wt::CharEncoding::UTF8);
		}
	}

	AggregatorApp::AggregatorApp (IProxyObject *ap, ICoreProxy_ptr cp,
			const Wt::WEnvironment& environment)
	: WApplication (environment)
	, AP_ (ap)
	, CP_ (cp)
	, ChannelsModel_ (new Wt::WStandardItemModel (this))
	, ChannelsFilter_ (new ReadChannelsFilter (this))
	, ItemsModel_ (new Wt::WStandardItemModel (this))
	{
		ChannelsFilter_->setSourceModel (ChannelsModel_);

		setTitle ("Aggregator WebAccess");
		setLoadingIndicator (new Wt::WOverlayLoadingIndicator ());

		SetupUI ();

		Q_FOREACH (Channel_ptr channel, AP_->GetAllChannels ())
		{
			const auto unreadCount = AP_->CountUnreadItems (channel->ChannelID_);

			auto title = new Wt::WStandardItem (ToW (channel->Title_));
			title->setIcon (Util::GetAsBase64Src (channel->Pixmap_).toStdString ());
			title->setData (channel->ChannelID_, ChannelRole::CID);
			title->setData (channel->FeedID_, ChannelRole::FID);
			title->setData (unreadCount, ChannelRole::UnreadCount);

			auto unread = new Wt::WStandardItem (ToW (QString::number (unreadCount)));

			ChannelsModel_->appendRow ({ title, unread });
		}
	}

	void AggregatorApp::HandleChannelClicked (const Wt::WModelIndex& idx)
	{
		ItemsModel_->clear ();
		ItemView_->setText (Wt::WString ());

		const IDType_t& cid = boost::any_cast<IDType_t> (idx.data (ChannelRole::CID));
		Q_FOREACH (Item_ptr item, AP_->GetChannelItems (cid))
		{
			if (!item->Unread_)
				continue;

			auto title = new Wt::WStandardItem (ToW (item->Title_));
			title->setData (item->ItemID_, ItemRole::IID);
			title->setData (item->ChannelID_, ItemRole::ParentCh);
			title->setData (item->Unread_, ItemRole::IsUnread);

			auto date = new Wt::WStandardItem (ToW (item->PubDate_.toString ()));

			ItemsModel_->appendRow ({ title, date });
		}
	}

	void AggregatorApp::SetupUI ()
	{
		auto rootLay = new Wt::WBoxLayout (Wt::WBoxLayout::LeftToRight);
		root ()->setLayout (rootLay);

		auto leftPaneLay = new Wt::WBoxLayout (Wt::WBoxLayout::TopToBottom);

		auto showReadChannels = new Wt::WCheckBox (ToW (QObject::tr ("Include read channels")));
		showReadChannels->setToolTip (ToW (QObject::tr ("Also display channels that have no unread items.")));
		showReadChannels->setChecked (false);
		showReadChannels->checked ().connect ([ChannelsFilter_] (Wt::NoClass) { ChannelsFilter_->SetHideRead (false); });
		showReadChannels->unChecked ().connect ([ChannelsFilter_] (Wt::NoClass) { ChannelsFilter_->SetHideRead (true); });
		leftPaneLay->addWidget (showReadChannels);

		auto channelsTree = new Wt::WTreeView ();
		channelsTree->setModel (ChannelsFilter_);
		leftPaneLay->addWidget (channelsTree);

		channelsTree->clicked ().connect (this, &AggregatorApp::HandleChannelClicked);

		rootLay->addLayout (leftPaneLay);

		auto rightPaneLay = new Wt::WBoxLayout (Wt::WBoxLayout::TopToBottom);

		auto itemsTree = new Wt::WTreeView ();
		itemsTree->setModel (ItemsModel_);
		rightPaneLay->addWidget (itemsTree);

		ItemView_ = new Wt::WText ();
		rightPaneLay->addWidget (ItemView_);
		rootLay->addLayout (rightPaneLay);
	}
}
}
}
