/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "deliciousservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <util/xpc/util.h>
#include <util/sll/prelude.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/iaccount.h>
#include "deliciousauthwidget.h"
#include "deliciousapi.h"
#include "deliciousaccount.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	DeliciousService::DeliciousService (ICoreProxy_ptr proxy)
	: CoreProxy_ (std::move (proxy))
	, DeliciousApi_ (new DeliciousApi)
	{
	}

	void DeliciousService::Prepare ()
	{
		RestoreAccounts ();
	}

	IBookmarksService::Features DeliciousService::GetFeatures () const
	{
		return FNone;
	}

	QObject* DeliciousService::GetQObject ()
	{
		return this;
	}

	QString DeliciousService::GetServiceName () const
	{
		return "Delicious";
	}

	QIcon DeliciousService::GetServiceIcon () const
	{
		return QIcon ("lcicons:/poshuku/onlinebookmarks:delicious/resources/images/delicious.png");
	}

	QWidget* DeliciousService::GetAuthWidget ()
	{
		return new DeliciousAuthWidget ();
	}

	void DeliciousService::CheckAuthData (const QVariantMap& map)
	{
		const QString& login = map ["Login"].toString ();
		const QString& password = map ["Password"].toString ();

		if (login.isEmpty () || password.isEmpty ())
			return;

		Request req;
		req.Type_ = OTAuth;
		req.Login_ = login;
		req.Password_ = password;
		req.Count_ = 0;
		req.Current_ = 0;

		SendRequest (DeliciousApi_->GetAuthUrl ().arg (login, password),
				QByteArray (), req);
	}

	void DeliciousService::RegisterAccount (const QVariantMap&)
	{
	}

	void DeliciousService::UploadBookmarks (QObject *accObj, const QVariantList& bookmarks)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
			<< "isn't an IAccount object"
			<< accObj;
			return;
		}

		int i = 0;
		for (const auto& var : bookmarks)
		{
			Request req;
			req.Type_ = OTUpload;
			req.Login_ = account->GetLogin ();
			req.Password_ = account->GetPassword ();
			req.Count_ = bookmarks.count ();
			req.Current_ = i;

			SendRequest (DeliciousApi_->GetUploadUrl ()
					.arg (account->GetLogin(), account->GetPassword ()),
						DeliciousApi_->GetUploadPayload (var),
						req );
		}
	}

	void DeliciousService::DownloadBookmarks (QObject *accObj, const QDateTime& from)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
			<< "isn't an IAccount object"
			<< accObj;
			return;
		}

		Request req;
		req.Type_ = OTDownload;
		req.Login_ = account->GetLogin ();
		req.Password_ = account->GetPassword ();
		req.Count_ = 0;
		req.Current_ = 0;
		Account2ReplyContent_ [account].clear ();

		SendRequest (DeliciousApi_->GetDownloadUrl ()
				.arg (account->GetLogin (), account->GetPassword ()),
					DeliciousApi_->GetDownloadPayload (from),
					req);
	}

	DeliciousAccount* DeliciousService::GetAccountByName (const QString& name)
	{
		for (const auto account : Accounts_)
			if (account->GetLogin () == name)
				return account;

		return nullptr;
	}

	void DeliciousService::SendRequest (const QString& urlString,
			const QByteArray& payload, const Request& req)
	{
		QUrl url (urlString);
		QNetworkRequest request (url);
		QNetworkReply *reply =  CoreProxy_->GetNetworkAccessManager ()->
				post (request, payload);

		Reply2Request_ [reply] = req;

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
						<< "undeserializable account"
						<< i;
				continue;
			}
			Accounts_ << acc;
		}

		if (!Accounts_.isEmpty ())
			emit accountAdded (Util::Map (Accounts_, &DeliciousAccount::GetQObject));
	}

	void DeliciousService::getReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "isn't a QNetworkReply";
			return;
		}

		if (Reply2Request_ [reply].Type_ == OTDownload)
		{
			DeliciousAccount *account = GetAccountByName (Reply2Request_ [reply].Login_);
			if (account)
			{
				const QVariantList& downloadedBookmarks = DeliciousApi_->
						ParseDownloadReply (Account2ReplyContent_ [account]);
				if (!downloadedBookmarks.isEmpty ())
				{
					account->AppendDownloadedBookmarks (downloadedBookmarks);
					account->SetLastDownloadDateTime (QDateTime::currentDateTime ());
					emit gotBookmarks (account, downloadedBookmarks);
				}
			}
		}
		reply->deleteLater ();
	}

	void DeliciousService::readyReadReply ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "isn't a QNetworkReply";
			return;
		}

		QByteArray result = reply->readAll ();
		Entity e;
		auto priority = Priority::Info;
		QString msg;
		switch (Reply2Request_ [reply].Type_)
		{
		case OTAuth:
			if (DeliciousApi_->ParseAuthReply (result))
			{
				auto account = new DeliciousAccount (Reply2Request_ [reply].Login_, this);
				account->SetPassword (Reply2Request_ [reply].Password_);
				Accounts_ << account;
				saveAccounts ();
				emit accountAdded (QObjectList () <<  account->GetQObject ());
				msg = tr ("Authentication successfull.");
				priority = Priority::Info;
			}
			else
			{
				msg = tr ("Invalid login or password.");
				priority = Priority::Warning;
			}
			e = Util::MakeNotification ("OnlineBookmarks",
					msg,
					priority);
			break;
		case OTUpload:
			if (DeliciousApi_->ParseUploadReply (result))
			{
				if (Reply2Request_ [reply].Count_ == Reply2Request_ [reply].Current_ + 1)
				{
					msg = tr ("Bookmarks were sent to Del.icio.us.");
					priority = Priority::Info;
					DeliciousAccount *account = GetAccountByName (Reply2Request_ [reply].Login_);
					if (account)
						account->SetLastUploadDateTime (QDateTime::currentDateTime ());
					emit bookmarksUploaded ();
				}
			}
			else
			{
				msg = tr ("Error sending bookmarks to Del.icio.us.");
				priority = Priority::Warning;
			}
			e = Util::MakeNotification ("OnlineBookmarks",
					msg,
					priority);
			break;
		case OTDownload:
			Account2ReplyContent_ [GetAccountByName (Reply2Request_ [reply].Login_)]
					.append (result);
			break;
		}
		if (!msg.isEmpty ())
			CoreProxy_->GetEntityManager ()->HandleEntity (e);
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
		auto account = qobject_cast<DeliciousAccount*> (accObj);
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
