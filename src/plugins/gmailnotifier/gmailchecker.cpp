/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gmailchecker.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDateTime>
#include <QTimer>
#include <QTimeZone>
#include <QDomDocument>
#include <QtDebug>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace GmailNotifier
{
	GmailChecker::GmailChecker (QObject *parent)
	: QObject (parent)
	, Reply_ (0)
	, TimeOutTimer_ (new QTimer (this))
	, Failed_ (false)
	{
		Init ();

		TimeOutTimer_->setInterval (60000);
		TimeOutTimer_->setSingleShot (true);
		connect (TimeOutTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (timeOut ()));
	}

	void GmailChecker::SetAuthSettings (const QString& login, const QString& passwd)
	{
		Username_ = login;
		Password_ = passwd;

		ReInit ();
	}

	void GmailChecker::Init ()
	{
		QNAM_ = new QNetworkAccessManager (this);
		connect (QNAM_,
				SIGNAL (authenticationRequired (QNetworkReply*, QAuthenticator*)),
				this,
				SLOT (httpAuthenticationRequired (QNetworkReply*, QAuthenticator*)));
	}

	void GmailChecker::ReInit ()
	{
		timeOut ();
		QNAM_->deleteLater ();
		QNAM_ = 0;
		Init ();
	}

	void GmailChecker::checkNow ()
	{
		if (!XmlSettingsManager::Instance ().property ("CheckingEnabled").toBool ())
			return;

		const bool isFirstRun = XmlSettingsManager::Instance ().Property ("FirstRun", true).toBool ();
		XmlSettingsManager::Instance ().setProperty ("FirstRun", false);
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

		Reply_ = QNAM_->get (QNetworkRequest (QUrl ("https://mail.google.com/mail/feed/atom")));
		connect (Reply_,
				SIGNAL (finished ()),
				this,
				SLOT (httpFinished ()));
		TimeOutTimer_->start ();

		emit waitMe ();
	}

	void GmailChecker::ParseData (const QString& data)
	{

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse XML data:"
					<< data;
			return;
		}

		QList<ConvInfo> result;

		const auto& root = doc.documentElement ();
		auto entry = root.firstChildElement ("entry");
		auto text = [&entry] (const QString& elemName)
			{ return entry.firstChildElement (elemName).text (); };
		auto localDate = [&text] (const QString& name)
		{
			return QDateTime::fromString (text (name), Qt::ISODate).toTimeZone (QTimeZone::UTC).toLocalTime ();
		};

		while (!entry.isNull ())
		{
			const auto& author = entry.firstChildElement ("author");

			ConvInfo info =
			{
				text ("title"),
				text ("summary"),
				entry.firstChildElement ("link").attribute ("href"),
				localDate ("issued"),
				localDate ("modified"),
				author.firstChildElement ("name").text (),
				author.firstChildElement ("email").text ()
			};
			result << info;

			entry = entry.nextSiblingElement ("entry");
		}

		emit gotConversations (result);
	}

	void GmailChecker::httpFinished ()
	{
		emit canContinue ();

		if (Reply_->error ())
		{
			qWarning () << Q_FUNC_INFO
					<< "reply error:"
					<< Reply_->errorString ()
					<< Reply_->error ();
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
			ParseData (QString::fromUtf8 (Reply_->readAll ()));

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

		emit canContinue ();
	}
}
}
