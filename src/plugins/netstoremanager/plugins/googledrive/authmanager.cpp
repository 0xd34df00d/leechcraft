/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "authmanager.h"
#include "core.h"
#include <QInputDialog>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <util/util.h>
#include <qjson/parser.h>

namespace LeechCraft
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
		emit gotEntity (e);

		InputDialog_ = new QInputDialog;
		Dialog2Account_ [InputDialog_] = acc;
		connect (InputDialog_,
				SIGNAL (finished (int)),
				this,
				SLOT (handleDialogFinished (int)));

		InputDialog_->setLabelText (tr ("Enter account verification code:"));
		InputDialog_->setWindowTitle (tr ("Account configuration"));
		InputDialog_->setWindowModality (Qt::NonModal);
		InputDialog_->setTextEchoMode (QLineEdit::Normal);

		InputDialog_->show ();
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
		InputDialog_->deleteLater ();
		Account *acc = Dialog2Account_.take (InputDialog_);
		std::shared_ptr<void> guard (static_cast<void*> (0),
				[this] (void*) { InputDialog_ = 0; });

		if (code == QDialog::Rejected)
			return;

		if (InputDialog_ &&
				InputDialog_->textValue ().isEmpty ())
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

		QByteArray data = reply->readAll ();

		QJson::Parser parser;
		bool ok;
		QVariant res = parser.parse (data, &ok);
		if (!ok)
			return;

		QVariantMap map = res.toMap ();

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
