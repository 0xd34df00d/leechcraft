/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QCoreApplication>
#include "ui_channelslistdialog.h"

class QStandardItem;
class QStandardItemModel;
class QTimer;

namespace LC::Azoth::Acetamide
{
	class ChannelsListFilterProxyModel;
	class IrcServerHandler;
	struct ChannelsDiscoverInfo;

	class ChannelsListDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Acetamide::ChannelsListDialog)

		enum Columns
		{
			ChannelName,
			ParticipantsCount,
			Subject
		};

		Ui::ChannelsListDialog Ui_;
		QList<QList<QStandardItem*>> Buffer_;
	public:
		explicit ChannelsListDialog (IrcServerHandler *ish, QWidget *parent = nullptr);
	};
}

