/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelslistdialog.h"
#include <chrono>
#include <QTimer>
#include <QStandardItemModel>
#include "channelslistfilterproxymodel.h"
#include "ircserverhandler.h"
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	ChannelsListDialog::ChannelsListDialog (IrcServerHandler* ish, QWidget* parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		const auto bufferTimer = new QTimer { this };
		const auto filterModel = new ChannelsListFilterProxyModel { this };
		const auto model = new QStandardItemModel { this };

		model->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Users count"), tr ("Topic") });
		filterModel->setSourceModel (model);
		Ui_.ChannelsList_->setModel (filterModel);
		Ui_.ChannelsList_->setColumnWidth (ChannelName, fontMetrics ().horizontalAdvance (QStringLiteral ("#asomewhatlongchannelname")));
		Ui_.ChannelsList_->setColumnWidth (ParticipantsCount, fontMetrics ().horizontalAdvance (QStringLiteral ("99999")));
		Ui_.ChannelsList_->header ()->setStretchLastSection (true);

		connect (bufferTimer,
				&QTimer::timeout,
				[this, model]
				{
					for (const auto& row : Buffer_)
						model->appendRow (row);
					Buffer_.clear ();
				});

		connect (ish,
				&IrcServerHandler::gotChannelsBegin,
				this,
				[=]
				{
					model->removeRows (0, model->rowCount ());

					using namespace std::chrono_literals;
					bufferTimer->start (1s);
				});
		connect (ish,
				&IrcServerHandler::gotChannelsEnd,
				bufferTimer,
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

		connect (Ui_.Filter_,
				&QLineEdit::textChanged,
				filterModel,
				qOverload<const QString&> (&ChannelsListFilterProxyModel::setFilterRegExp));
		connect (Ui_.ChannelsList_,
				&QTreeView::activated,
				this,
				[ish] (const QModelIndex& index)
				{
					ish->JoinChannel ({
							.ServerName_ = ish->GetServerOptions ().ServerName_,
							.ChannelName_ = index.siblingAtColumn (Columns::ChannelName).data ().toString (),
						});
				});
	}
}
