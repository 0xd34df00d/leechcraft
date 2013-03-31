/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "channelslistdialog.h"
#include <QTimer>
#include <QStandardItemModel>
#include "channelslistfilterproxymodel.h"
#include "ircserverhandler.h"

namespace LeechCraft
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

		connect (BufferTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (appendRows ()));
	}

	void ChannelsListDialog::handleGotChannelsBegin ()
	{
		Model_->removeRows (0, Model_->rowCount ());
		Ui_.ChannelsList_->setEnabled (false);
		Ui_.Filter_->setEnabled (false);
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
		Ui_.ChannelsList_->setEnabled (true);
		Ui_.Filter_->setEnabled (true);
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
