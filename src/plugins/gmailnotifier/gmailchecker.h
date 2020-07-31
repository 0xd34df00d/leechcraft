/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC
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

