/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include "interfaces/blogique/iaccount.h"

class QTimer;

namespace LC
{
namespace Blogique
{
	class CommentsManager : public QObject
	{
		Q_OBJECT

		QTimer *CommentsCheckingTimer_;
		QSet<CommentEntry> RecentComments_;

	public:
		CommentsManager (QObject *parent = 0);

		QList<CommentEntry> GetComments () const;

	private slots:
		void checkForComments ();
		void handleCommentsCheckingChanged ();
		void handleCommentsCheckingTimerChanged ();
		void handleGotRecentComments (const QList<CommentEntry>& comments);
		void handleCommentsDeleted (const QList<qint64>& ids);

	signals:
		void commentsUpdated ();
	};
}
}
