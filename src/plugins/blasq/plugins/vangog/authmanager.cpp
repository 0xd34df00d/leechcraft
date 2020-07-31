/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authmanager.h"
#include <QInputDialog>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMainWindow>
#include <util/sll/parsejson.h>
#include <util/xpc/util.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "picasaaccount.h"

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	namespace
	{
		const QString ClientId ("844868161425.apps.googleusercontent.com");
		const QString ClientSecret ("l09HkM6nbPMEYcMdcdeGBdaV");
		const QString Scope ("https://picasaweb.google.com/data/");
		const QString ResponseType ("code");
		const QString RedirectUri ("urn:ietf:wg:oauth:2.0:oob");
	}

	AuthManager::AuthManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
	}

	void AuthManager::Auth (PicasaAccount *acc)
	{
		QUrl url (QString ("https://accounts.google.com/o/oauth2/auth?client_id=%1&scope=%2&response_type=%3&redirect_uri=%4")
				.arg (ClientId)
				.arg (Scope)
				.arg (ResponseType)
				.arg (RedirectUri));

		Entity e = Util::MakeEntity (url,
				QString (),
				FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (e);

		auto rootWM = Proxy_->GetRootWindowsManager ();
		InputDialog_ = new QInputDialog (rootWM->GetPreferredWindow (), Qt::Widget);
		Dialog2Account_ [InputDialog_] = acc;
		connect (InputDialog_,
				SIGNAL (finished (int)),
				this,
				SLOT (handleDialogFinished (int)));

		InputDialog_->setLabelText (tr ("A browser window will pop up with a request for "
				"permissions to access your Google account. Once you accept it, a "
				"verification code will appear. Enter that verification code in the box below:"));
		InputDialog_->setWindowTitle (tr ("Account configuration"));
		InputDialog_->setTextEchoMode (QLineEdit::Normal);

		InputDialog_->show ();
		InputDialog_->activateWindow ();
	}

	void AuthManager::RequestAuthToken (const QString& code, PicasaAccount *acc)
	{
		QNetworkRequest request (QUrl ("https://accounts.google.com/o/oauth2/token"));
		QString str = QString ("code=%1&client_id=%2&client_secret=%3&grant_type=%4&redirect_uri=%5")
				.arg (code)
				.arg (ClientId)
				.arg (ClientSecret)
				.arg ("authorization_code")
				.arg ("urn:ietf:wg:oauth:2.0:oob");

		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

		QNetworkReply *reply = Proxy_->GetNetworkAccessManager ()->
				post (request, str.toUtf8 ());
		Reply2Account_ [reply] = acc;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestAuthTokenFinished ()));
	}

	void AuthManager::handleDialogFinished (int code)
	{
		InputDialog_->deleteLater ();
		PicasaAccount *acc = Dialog2Account_.take (InputDialog_);
		std::shared_ptr<void> guard (nullptr,
				[this] (void*) { InputDialog_ = 0; });

		if (code == QDialog::Rejected)
			return;

		if (InputDialog_->textValue ().isEmpty ())
			return;

		RequestAuthToken (InputDialog_->textValue (), acc);
	}

	void AuthManager::handleRequestAuthTokenFinished ()
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		const auto acc = Reply2Account_.take (reply);
		reply->deleteLater ();

		const auto& map = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();
		if (map.isEmpty ())
			return;

		if (map.contains ("error"))
		{
			qWarning () << Q_FUNC_INFO
					<< "there is error in answer"
					<< map;
			return;
		}

		if (map.contains ("refresh_token"))
			acc->SetRefreshToken (map ["refresh_token"].toString ());

		emit authSuccess (acc);
	}

}
}
}
