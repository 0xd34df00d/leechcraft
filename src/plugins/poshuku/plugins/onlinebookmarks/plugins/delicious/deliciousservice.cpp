/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "deliciousservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <interfaces/iaccount.h>
#include "deliciousauthwidget.h"
#include "deliciousapi.h"
#include "deliciousaccount.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	DeliciousService::DeliciousService (ICoreProxy_ptr proxy)
	: CoreProxy_ (proxy)
	, DeliciousApi_ (new DeliciousApi)
	{
	}

	void DeliciousService::Prepare ()
	{
		RestoreAccounts ();
	}

	IBookmarksService::Features DeliciousService::GetFeatures () const
	{
		return 0;
	}

	QObject* DeliciousService::GetObject ()
	{
		return this;
	}

	QString DeliciousService::GetServiceName () const
	{
		return "Delicious";
	}

	QIcon DeliciousService::GetServiceIcon () const
	{
		return QIcon (":/plugins/poshuku/plugins/onlinebookmarks/plugins/delicious/resources/images/delicious.png");
	}

	QWidget* DeliciousService::GetAuthWidget ()
	{
		return new DeliciousAuthWidget ();
	}

	void DeliciousService::CheckAuthData (const QVariantMap& map)
	{
		const QString login = map ["Login"].toString ();
		const QString password = map ["Password"].toString ();
		bool oAuth = false;
		if (map.contains ("OAuth"))
			oAuth = map ["OAuth"].toBool ();
		
		if (login.isEmpty () || password.isEmpty ())
			return;

		SendRequest (DeliciousApi_->GetAuthUrl (),
				DeliciousApi_->GetAuthPayload (login, password));
	}

	void DeliciousService::RegisterAccount (const QVariantMap&)
	{
	}

	void DeliciousService::UploadBookmarks (IAccount *account, const QVariantList& bookmarks)
	{
		QByteArray uploadBookmarks = DeliciousApi_->GetUploadPayload (account->GetLogin(),
				account->GetPassword (), bookmarks);

		SendRequest (DeliciousApi_->GetUploadUrl (),
				uploadBookmarks);
	}

	void DeliciousService::DownloadBookmarks (IAccount *account, const QDateTime& from)
	{
		QByteArray downloadBookmarks = DeliciousApi_->GetDownloadPayload (account->GetLogin(),
				account->GetPassword (), from);

		SendRequest (DeliciousApi_->GetDownloadUrl (),
				downloadBookmarks);
	}

	void DeliciousService::SendRequest (const QString& urlString, const QByteArray& payload)
	{
		QUrl url (urlString);
		QNetworkRequest request (url);
		QNetworkReply *reply =  CoreProxy_->GetNetworkAccessManager ()->
				post (request, payload);

// 		Reply2Request_ [reply] = req;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (getReplyFinished ()));

		connect (reply,
				SIGNAL (readyRead ()),
				this,
				SLOT (readyReadReply ()));
	}

	void DeliciousService::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Poshuku_OnlineBookmarks_Delicious_Accounts");

		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings
					.value ("SerializedData").toByteArray ();

			DeliciousAccount *acc = DeliciousAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}
			Accounts_ << acc;
			emit accountAdded (acc);
		}
	}

	void DeliciousService::getReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "isn't a QNetworkReply";
			return;
		}
		reply->deleteLater ();
	}

	void DeliciousService::readyReadReply ()
	{

	}

	void DeliciousService::saveAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Poshuku_OnlineBookmarks_Delicious_Accounts");

		settings.beginWriteArray ("Accounts");

		for (int i = 0, size = Accounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					Accounts_.at (i)->Serialize ());
		}

		settings.endArray ();
		settings.sync ();
	}

	void DeliciousService::removeAccount (QObject *accObj)
	{
		DeliciousAccount *account = qobject_cast<DeliciousAccount*> (accObj);
		if (Accounts_.removeAll (account))
		{
			accObj->deleteLater ();
			saveAccounts ();
		}
	}

}
}
}
}
