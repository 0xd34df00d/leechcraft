/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QSet>
#include <QDateTime>

class QTimer;

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	class TypingManager : public QObject
	{
		Q_OBJECT

		QMap<QString, QDateTime> LastNotDates_;
		QTimer *ExpTimer_;

		QSet<QString> TypingTo_;
		QTimer *OutTimer_;
	public:
		TypingManager (QObject* = 0);

		void GotNotification (const QString&);

		void SetTyping (const QString&, bool);
	private slots:
		void checkExpires ();
		void sendOut ();
	signals:
		void startedTyping (const QString&);
		void stoppedTyping (const QString&);

		void needNotify (const QString&);
	};
}
}
}
}
