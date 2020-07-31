/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <QMap>
#include <QPointer>

namespace LC
{
namespace Azoth
{
	class UnreadQueueManager : public QObject
	{
		Q_OBJECT

		QList<QPointer<QObject>> Queue_;
		QSet<QPointer<QObject>> UnreadMessages_;

		QMap<QObject*, QPointer<QObject>> Entry2FirstUnread_;
	public:
		UnreadQueueManager (QObject* = 0);

		QObject* GetFirstUnreadMessage (QObject *entryObj) const;

		void AddMessage (QObject*);
		bool IsMessageRead (QObject*) const;
		void ShowNext ();
	public slots:
		void clearMessagesForEntry (QObject*);
	signals:
		void messagesCleared (QObject*);
	};
}
}
