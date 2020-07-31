/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <util/xpc/util.h>
#include <util/sll/parsejson.h>
#include <util/sll/util.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "account.h"
#include "core.h"

namespace LC
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	AuthManager::AuthManager (QObject *parent)
	: QObject (parent)
	, ClientId_ ("844868161425.apps.googleusercontent.com")
	, ClientSecret_ ("l09HkM6nbPMEYcMdcdeGBdaV")
	, Scope_ ("https://www.googleapis.com/auth/drive")
	, ResponseType_ ("code")
	, RedirectUri_ ("urn:ietf:wg:oauth:2.0:oob")
	{
	}

	void AuthManager::Auth (Account *acc)
	{
		QUrl url (QString ("https://accounts.google.com/o/oauth2/auth?client_id=%1&scope=%2&response_type=%3&redirect_uri=%4")
				.arg (ClientId_)
				.arg (Scope_)
				.arg (ResponseType_)
				.arg (RedirectUri_));

		Entity e = Util::MakeEntity (url,
				QString (),
				FromUserInitiated | OnlyHandle);
		Core::Instance ().SendEntity (e);

		auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		InputDialog_ = new QInputDialog (rootWM->GetPreferredWindow (), Qt::Widget);
		Dialog2Account_ [InputDialog_] = acc;
		connect (InputDialog_,
				SIGNAL (finished (int)),
				this,
				SLOT (handleDialogFinished (int)));

		InputDialog_->setLabelText (tr ("A browser window will pop up with a request for "
				"permissions to access your Google Drive account. Once you accept it, a "
				"verification code will appear. Enter that verification code in the box below:"));
		InputDialog_->setWindowTitle (tr ("Account configuration"));
		InputDialog_->setTextEchoMode (QLineEdit::Normal);

		InputDialog_->show ();
		InputDialog_->activateWindow ();
	}

	void AuthManager::RequestAuthToken (const QString& code, Account *acc)
	{
		QNetworkRequest request (QUrl ("https://accounts.google.com/o/oauth2/token"));
		QString str = QString ("code=%1&client_id=%2&client_secret=%3&grant_type=%4&redirect_uri=%5")
				.arg (code)
				.arg (ClientId_)
				.arg (ClientSecret_)
				.arg ("authorization_code")
				.arg ("urn:ietf:wg:oauth:2.0:oob");

		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, str.toUtf8 ());
		Reply2Account_ [reply] = acc;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestAuthTokenFinished ()));
	}

	void AuthManager::handleDialogFinished (int code)
	{
		if (!InputDialog_)
			return;

		InputDialog_->deleteLater ();
		Account *acc = Dialog2Account_.take (InputDialog_);
		const auto guard = Util::MakeScopeGuard ([this] { InputDialog_ = nullptr; });

		if (code == QDialog::Rejected)
			return;

		if (InputDialog_->textValue ().isEmpty ())
			return;

		RequestAuthToken (InputDialog_->textValue (), acc);
	}

	void AuthManager::handleRequestAuthTokenFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		Account *acc = Reply2Account_.take (reply);
		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		const auto& map = res.toMap ();

		if (map.contains ("error"))
			return;

		if (map.contains ("access_token"))
			acc->SetAccessToken (map ["access_token"].toString ());
		if (map.contains ("refresh_token"))
			acc->SetRefreshToken (map ["refresh_token"].toString ());
		acc->SetTrusted (true);

		emit authSuccess (acc);
	}

}
}
}
