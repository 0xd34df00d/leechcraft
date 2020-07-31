/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <interfaces/core/icoreproxy.h>
#include "convinfo.h"

namespace LC
{
namespace GmailNotifier
{
	class MailListView;

	class QuarkManager : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (int msgCount READ GetMsgCount NOTIFY msgCountChanged)

		ICoreProxy_ptr Proxy_;

		ConvInfos_t Infos_;
		QPointer<MailListView> MailListView_;
	public:
		QuarkManager (ICoreProxy_ptr, QObject* = 0);

		int GetMsgCount () const;
	public slots:
		void showMailList (int x, int y, const QRect&);
		void handleConversations (const ConvInfos_t&);
	signals:
		void msgCountChanged ();
	};
}
}
