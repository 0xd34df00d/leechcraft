/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelslistdialog.h"
#include <QTimer>
#include <QStandardItemModel>
#include "channelslistfilterproxymodel.h"
#include "ircserverhandler.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ChannelsListDialog::ChannelsListDialog (IrcServerHandler* ish, QWidget* parent)
	: QDialog (parent)
	, ISH_ (ish)
	, BufferTimer_ (new QTimer (this))
	, FilterProxyModel_ (new ChannelsListFilterProxyModel (this))
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Users count"), tr ("Topic") });
		FilterProxyModel_->setSourceModel (Model_);
		Ui_.ChannelsList_->setModel (FilterProxyModel_);
		Ui_.ChannelsList_->setColumnWidth (ChannelName, 200);
		Ui_.ChannelsList_->setColumnWidth (ParticipantsCount, 50);
		Ui_.ChannelsList_->header ()->setStretchLastSection (true);

		connect (BufferTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (appendRows ()));
	}

	void ChannelsListDialog::handleGotChannelsBegin ()
	{
		Model_->removeRows (0, Model_->rowCount ());
		BufferTimer_->start (1000);
	}

	void ChannelsListDialog::handleGotChannels (const ChannelsDiscoverInfo& info)
	{
		QStandardItem *name = new QStandardItem (info.ChannelName_);
		name->setEditable (false);
		QStandardItem *count = new QStandardItem (QString::number (info.UsersCount_));
		count->setEditable (false);
		QStandardItem *topic = new QStandardItem (info.Topic_);
		topic->setEditable (false);
		Buffer_.append ({ name, count, topic });
	}

	void ChannelsListDialog::handleGotChannelsEnd ()
	{
		BufferTimer_->stop ();
	}

	void ChannelsListDialog::appendRows ()
	{
		auto list = Buffer_;
		Buffer_.clear ();

		for (const auto& row : list)
			Model_->appendRow (row);
	}

	void ChannelsListDialog::on_Filter__textChanged (const QString& text)
	{
		FilterProxyModel_->setFilterRegExp (text);
	}

	void ChannelsListDialog::on_ChannelsList__doubleClicked (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		QModelIndex idx = index.sibling (index.row (), 0);
		ChannelOptions opts;
		opts.ChannelName_ = idx.data ().toString ();
		opts.ServerName_ = ISH_->GetServerOptions ().ServerName_;
		ISH_->JoinChannel (opts);
	}
}
}
}
