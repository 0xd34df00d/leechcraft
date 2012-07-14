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
#include <Wt/WTableView>
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
	, ChannelsModel_ (new Wt::WStandardItemModel (0, 2, this))
	, ChannelsFilter_ (new ReadChannelsFilter (this))
	, ItemsModel_ (new Wt::WStandardItemModel (0, 2, this))
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
			unread->setData (channel->ChannelID_, ChannelRole::CID);

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
			title->setData (ToW (item->Link_), ItemRole::Link);
			title->setData (ToW (item->Description_), ItemRole::Text);

			auto date = new Wt::WStandardItem (ToW (item->PubDate_.toString ()));

			ItemsModel_->insertRow (0, { title, date });
		}

		ItemsTable_->setColumnWidth (0, Wt::WLength (500, Wt::WLength::Pixel));
		ItemsTable_->setColumnWidth (1, Wt::WLength (180, Wt::WLength::Pixel));
	}

	void AggregatorApp::HandleItemClicked (const Wt::WModelIndex& idx)
	{
		auto titleIdx = idx.model ()->index (idx.row (), 0);
		auto pubDate = idx.model ()->index (idx.row (), 1);
		auto text = Wt::WString ("<div><a href='{1}' target='_blank'>{2}</a><br />{3}<br /><hr/>{4}</div>")
				.arg (boost::any_cast<Wt::WString> (titleIdx.data (ItemRole::Link)))
				.arg (boost::any_cast<Wt::WString> (titleIdx.data ()))
				.arg (boost::any_cast<Wt::WString> (pubDate.data ()))
				.arg (boost::any_cast<Wt::WString> (titleIdx.data (ItemRole::Text)));
		ItemView_->setText (text);
	}

	void AggregatorApp::SetupUI ()
	{
		auto rootLay = new Wt::WBoxLayout (Wt::WBoxLayout::LeftToRight);
		root ()->setLayout (rootLay);

		auto leftPaneLay = new Wt::WBoxLayout (Wt::WBoxLayout::TopToBottom);
		rootLay->addLayout (leftPaneLay, 2);

		auto showReadChannels = new Wt::WCheckBox (ToW (QObject::tr ("Include read channels")));
		showReadChannels->setToolTip (ToW (QObject::tr ("Also display channels that have no unread items.")));
		showReadChannels->setChecked (false);
		showReadChannels->checked ().connect ([ChannelsFilter_] (Wt::NoClass) { ChannelsFilter_->SetHideRead (false); });
		showReadChannels->unChecked ().connect ([ChannelsFilter_] (Wt::NoClass) { ChannelsFilter_->SetHideRead (true); });
		leftPaneLay->addWidget (showReadChannels);

		auto channelsTree = new Wt::WTreeView ();
		channelsTree->setModel (ChannelsFilter_);
		channelsTree->clicked ().connect (this, &AggregatorApp::HandleChannelClicked);
		channelsTree->setAlternatingRowColors (true);
		leftPaneLay->addWidget (channelsTree, 1, Wt::AlignTop);

		auto rightPaneLay = new Wt::WBoxLayout (Wt::WBoxLayout::TopToBottom);
		rootLay->addLayout (rightPaneLay, 7);

		ItemsTable_ = new Wt::WTableView ();
		ItemsTable_->setModel (ItemsModel_);
		ItemsTable_->clicked ().connect (this, &AggregatorApp::HandleItemClicked);
		ItemsTable_->setAlternatingRowColors (true);
		ItemsTable_->setWidth (Wt::WLength (100, Wt::WLength::Percentage));
		rightPaneLay->addWidget (ItemsTable_, 2, Wt::AlignJustify);

		ItemView_ = new Wt::WText ();
		rightPaneLay->addWidget (ItemView_, 5);
	}
}
}
}
