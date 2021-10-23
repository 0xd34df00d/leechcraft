/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "localtypes.h"
#include "ui_channelslistdialog.h"

class QStandardItem;
class QStandardItemModel;
class QTimer;

namespace LC
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

		enum Columns
		{
			ChannelName,
			ParticipantsCount,
			Subject
		};

		Ui::ChannelsListDialog Ui_;
		IrcServerHandler *ISH_;
		QList<QList<QStandardItem*>> Buffer_;
		QTimer *BufferTimer_;
		ChannelsListFilterProxyModel *FilterProxyModel_;
		QStandardItemModel *Model_;

	public:
		explicit ChannelsListDialog (IrcServerHandler *ish, QWidget *parent = 0);

	private slots:
		void appendRows ();
		void on_Filter__textChanged (const QString& text);
		void on_ChannelsList__doubleClicked (const QModelIndex& index);
	};
}
}
}

