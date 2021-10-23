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

		connect (ish,
				&IrcServerHandler::gotChannelsBegin,
				this,
				[this]
				{
					Model_->removeRows (0, Model_->rowCount ());

					using namespace std::chrono_literals;
					BufferTimer_->start (1s);
				});
		connect (ish,
				&IrcServerHandler::gotChannelsEnd,
				BufferTimer_,
				&QTimer::stop);
		connect (ish,
				&IrcServerHandler::gotChannels,
				this,
				[this] (const ChannelsDiscoverInfo& info)
				{
					Buffer_.append ({
							new QStandardItem { info.ChannelName_ },
							new QStandardItem { QString::number (info.UsersCount_) },
							new QStandardItem { info.Topic_ },
						});
					for (const auto item : Buffer_.last ())
						item->setEditable (false);
				});
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
