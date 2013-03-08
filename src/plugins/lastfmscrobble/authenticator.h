/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

class QNetworkAccessManager;
class QDomDocument;

namespace LeechCraft
{
struct Entity;

namespace Lastfmscrobble
{
	class Authenticator : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;
		bool IsAuthenticated_;
	public:
		Authenticator (QNetworkAccessManager*, QObject* = 0);

		void Init ();
		bool IsAuthenticated () const;
	private:
		void FeedPassword (bool);
		bool CheckError (const QDomDocument&);
	private slots:
		void getSessionKey ();
		void handleAuth ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);

		void authenticated ();
	};
}
}
