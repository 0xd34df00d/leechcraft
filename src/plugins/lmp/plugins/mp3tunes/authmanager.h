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

class QNetworkAccessManager;

namespace LeechCraft
{
struct Entity;

namespace LMP
{
namespace MP3Tunes
{
	class AuthManager : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;

		QMap<QString, QString> Login2Sid_;
		QSet<QString> FailedAuth_;
	public:
		AuthManager (QNetworkAccessManager*, QObject* = 0);

		QString GetSID (const QString&);
	private slots:
		void handleAuthReplyFinished ();
		void handleAuthReplyError ();
	signals:
		void sidReady (const QString&);
		void sidError (const QString&, const QString&);

		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}
