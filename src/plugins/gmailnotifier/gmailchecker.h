/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
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

#ifndef PLUGINS_GMAILNOTIFIER_GMAILCHECKER_H
#define PLUGINS_GMAILNOTIFIER_GMAILCHECKER_H
#include <QObject>
#include <QString>
#include "convinfo.h"

class QNetworkAccessManager;
class QNetworkReply;
class QAuthenticator;
class QTimer;

namespace LeechCraft
{
namespace GmailNotifier
{
	class GmailChecker: public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *QNAM_;
		QNetworkReply *Reply_;
		QTimer *TimeOutTimer_;

		QString Username_;
		QString Password_;

		bool Failed_;
	public:
		GmailChecker(QObject* = 0);
		void SetAuthSettings (const QString& login, const QString& passwd);

		void Init ();
		void ReInit ();
	private:
		void ParseData (const QString&);
	public slots:
		void checkNow ();
	private slots:
		void httpFinished ();
		void httpAuthenticationRequired (QNetworkReply *networkReply, QAuthenticator *authenticator);
		void timeOut ();
	signals:
		void gotConversations (const ConvInfos_t&);
		void anErrorOccupied (const QString& title, const QString& msg);

		void waitMe ();
		void canContinue ();
	};
}
}

#endif

