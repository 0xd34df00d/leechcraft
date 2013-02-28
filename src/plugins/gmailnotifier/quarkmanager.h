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

#pragma once

#include <QObject>
#include <QPointer>
#include "convinfo.h"
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
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
