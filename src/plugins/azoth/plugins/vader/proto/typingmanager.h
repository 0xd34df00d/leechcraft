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
#include <QMap>
#include <QSet>
#include <QDateTime>

class QTimer;

namespace LeechCraft
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
