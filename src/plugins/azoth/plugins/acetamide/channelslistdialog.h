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

#pragma once

#include <QDialog>
#include "localtypes.h"
#include "ui_channelslistdialog.h"

class QStandardItem;
class QStandardItemModel;
class QTimer;

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class ChannelsListFilterProxyModel;
	class IrcServerHandler;

	class ChannelsListDialog : public QDialog
	{
		Q_OBJECT

		Ui::ChannelsListDialog Ui_;
		IrcServerHandler *ISH_;
		QList<QList<QStandardItem*>> Buffer_;
		QTimer *BufferTimer_;
		ChannelsListFilterProxyModel *FilterProxyModel_;
		QStandardItemModel *Model_;

	public:
		explicit ChannelsListDialog (IrcServerHandler *ish, QWidget *parent = 0);

	public slots:
		void handleGotChannelsBegin ();
		void handleGotChannels (const ChannelsDiscoverInfo& info);
		void handleGotChannelsEnd ();
	private slots:
		void appendRows ();
		void on_Filter__textChanged (const QString& text);
		void on_ChannelsList__doubleClicked (const QModelIndex& index);
	};
}
}
}

