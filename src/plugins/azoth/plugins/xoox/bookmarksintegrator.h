/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>

class QXmppBookmarkManager;
class QXmppBookmarkSet;

namespace LC::Azoth::Xoox
{
	class ClientConnection;
	class GlooxAccount;

	class BookmarksIntegrator : public QObject
	{
		GlooxAccount& Acc_;
		ClientConnection& Conn_;

		QXmppBookmarkManager& Mgr_;

		struct JoinQueueItem
		{
			bool AsAutojoin_;
			QString RoomJID_;
			QString Nickname_;
		};
		QList<JoinQueueItem> JoinQueue_;
	public:
		BookmarksIntegrator (QXmppBookmarkManager&, ClientConnection&, GlooxAccount&, QObject* = nullptr);

		QVariantList GetBookmarkedMUCs () const;
		void SetBookmarkedMUCs (const QVariantList&);
	private:
		void HandleBookmarksReceived (const QXmppBookmarkSet&);
		void HandleAutojoinQueue ();
	};
}
