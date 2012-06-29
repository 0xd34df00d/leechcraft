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

#include "gmailchecker.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDateTime>
#include <QTimer>
#include <QDomDocument>
#include <QtDebug>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace GmailNotifier
{
	GmailChecker::GmailChecker (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Reply_ (0)
	, TimeOutTimer_ (new QTimer (this))
	, Failed_ (false)
	, Proxy_ (proxy)
	{
		TimeOutTimer_->setInterval (60000);
		TimeOutTimer_->setSingleShot (true);
		connect (TimeOutTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (timeOut ()));

		connect (Proxy_->GetNetworkAccessManager (),
				SIGNAL (authenticationRequired (QNetworkReply*, QAuthenticator*)),
				this,
				SLOT (httpAuthenticationRequired (QNetworkReply*, QAuthenticator*)));
	}

	void GmailChecker::SetAuthSettings (const QString& login, const QString& passwd)
	{
		Username_ = login;
		Password_ = passwd;

		ReInit ();
	}

	void GmailChecker::ReInit ()
	{
		timeOut ();
	}

	void GmailChecker::checkNow ()
	{
		if (!XmlSettingsManager::Instance ()->property ("CheckingEnabled").toBool ())
			return;

		const bool isFirstRun = XmlSettingsManager::Instance ()->
				Property ("FirstRun", true).toBool ();
		XmlSettingsManager::Instance ()->setProperty ("FirstRun", false);
		if (Username_.isEmpty ())
		{
			if (isFirstRun)
				emit anErrorOccupied ("Gmail Notifier",
						tr ("Username for the GMail checker isn't set. "
							"You can enable it in GMail Notifier settings."));
			return;
		}

		if (Password_.isEmpty ())
		{
			emit anErrorOccupied ("Gmail Notifier", tr ("Password isn't set"));
			return;
		}

		Failed_ = false;
		Data_.clear ();

		Reply_ = Proxy_->GetNetworkAccessManager ()->
				get (QNetworkRequest (QUrl ("https://mail.google.com/mail/feed/atom")));
		connect (Reply_,
				SIGNAL (finished ()),
				this,
				SLOT (httpFinished ()));
		TimeOutTimer_->start ();

		emit waitMe ();
	}

	void GmailChecker::ParseData ()
	{
		QString error = tr ("Error");
		error.prepend ("Gmail Notifier: ");

		QDomDocument doc;
		doc.setContent (Data_);

		QDomElement root = doc.documentElement ();
		bool ok = false;
		const int fullCount = root.firstChildElement ("fullcount").text ().toInt (&ok);
		if (!ok)
		{
			emit anErrorOccupied (error, tr ("Cannot parse XML data"));
			return;
		}
		if (!fullCount)
			return;

		QString title = root.firstChildElement ("title")
				.text ().replace ("Gmail - ", "Gmail Notifier: ");
		int i = 0;
		const int fullShow = XmlSettingsManager::Instance ()->
				property ("ShowLastNMessages").toInt ();
		QString result;
		for (QDomElement elem = root.firstChildElement ("entry");
				!elem.isNull () && i < fullShow; elem = elem.nextSiblingElement ("entry"), ++i)
		{
			const QString& dateText = elem.firstChildElement ("issued").text ();
			const QDateTime& dt = QDateTime::fromString (dateText, Qt::ISODate);
			const QString& subject = elem.firstChildElement ("title").text ().isEmpty () ?
					tr ("No subject") :
					elem.firstChildElement ("title").text ();
			const QString& summary = elem.firstChildElement ("summary").text ().isEmpty () ?
					tr ("No content") :
					elem.firstChildElement ("summary").text ();
			result += QString::fromUtf8 ("<p><font color=\"#004C00\">\302\273</font>&nbsp;<a href=\"");
			result += elem.firstChildElement ("link").attribute ("href") + "\">";
			result += subject + "</a> " + tr ("from") + " ";
			result += "<a href=\"https://mail.google.com/mail?extsrc=mailto&url=mailto:";
			result += elem.firstChildElement ("author").firstChildElement ("email").text () + "\">";
			result += elem.firstChildElement ("author").firstChildElement ("name").text () + "</a><br/>";
			result += tr ("at") + " " + dt.toLocalTime ().toString (Qt::SystemLocaleLongDate);
			result += "</p><p class=\"additionaltext\">";
			result += summary + "</p>";
		}
		if (fullCount > fullShow)
			result += "<p><em>&hellip;" +
					tr ("and %1 more").arg (fullCount - fullShow) +
					"</em></p>";
		emit newConversationsAvailable (title, result);
	}

	void GmailChecker::httpFinished ()
	{
		emit canContinue ();

		if (Reply_->error ())
		{
			QString error = tr ("Error");
			error.prepend ("Gmail Notifier: ");

			if (Reply_->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt () == 401)
				emit anErrorOccupied (error.append (" 401"),
						tr ("The username or password is incorrect"));
			else if (Reply_->error() == QNetworkReply::OperationCanceledError) // When timed out
				emit anErrorOccupied (error, tr ("Connection timeout"));
			else
				emit anErrorOccupied (error, Reply_->errorString ());
		}
		else
		{
			Data_ = QString::fromUtf8 (Reply_->readAll ());
			ParseData ();
		}

		Reply_->deleteLater ();
		Reply_ = 0;
	}

	void GmailChecker::httpAuthenticationRequired (QNetworkReply*, QAuthenticator *authenticator)
	{
		if (!Failed_)
		{
			authenticator->setUser (Username_);
			authenticator->setPassword (Password_);
		}

		Failed_ = true;
	}

	void GmailChecker::timeOut ()
	{
		if (Reply_)
			Reply_->abort ();
	}
}
}
