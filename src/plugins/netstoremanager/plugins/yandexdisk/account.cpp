/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "account.h"
#include <QInputDialog>
#include <QNetworkRequest>
#include <QtDebug>
#include <util/passutils.h>
#include <util/util.h>
#include "yandexdisk.h"
#include "uploadmanager.h"
#include "authmanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	Account::Account (Plugin *plugin)
	: QObject (plugin)
	, Plugin_ (plugin)
	, AM_ (new AuthManager (this))
	{
	}

	QByteArray Account::Serialize () const
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
			<< Name_
			<< Login_;
		return result;
	}

	Account_ptr Account::Deserialize (const QByteArray& data, Plugin *parent)
	{
		QDataStream str (data);
		quint8 version = 0;
		str >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return Account_ptr ();
		}

		Account_ptr acc (new Account (parent));
		str >> acc->Name_
			>> acc->Login_;
		return acc;
	}

	AuthManager* Account::GetAuthManager () const
	{
		return AM_;
	}

	QString Account::GetLogin () const
	{
		return Login_;
	}

	QString Account::GetPassword ()
	{
		if (Login_.isEmpty ())
			return QString ();

		return Util::GetPassword ("org.LeechCraft.NetStoreManager.YandexDisk/" + Login_,
				tr ("Enter password for Yandex.Disk account %1 with login %2:")
					.arg (Name_)
					.arg (Login_),
				Plugin_);
	}

	bool Account::ExecConfigDialog ()
	{
		const QString& login = QInputDialog::getText (0,
				tr ("Account configuration"),
				tr ("Enter account login:"),
				QLineEdit::Normal,
				Login_);
		if (login.isEmpty ())
			return false;

		Login_ = login;
		return true;
	}

	void Account::SetAccountName (const QString& name)
	{
		Name_ = name;
	}

	QString Account::GetAccountName () const
	{
		return Name_;
	}

	QObject* Account::GetParentPlugin () const
	{
		return Plugin_;
	}

	QObject* Account::GetObject ()
	{
		return this;
	}

	AccountFeatures Account::GetAccountFeatures () const
	{
		return AccountFeature::FileListings | AccountFeature::ProlongateFiles;
	}

	void Account::Upload (const QString& path)
	{
		auto mgr = new UploadManager (path, this);
		connect (mgr,
				SIGNAL (statusChanged (QString, QString)),
				this,
				SIGNAL (upStatusChanged (QString, QString)));
		connect (mgr,
				SIGNAL (gotError (QString, QString)),
				this,
				SIGNAL (upError (QString, QString)));
		connect (mgr,
				SIGNAL (gotUploadURL (QUrl, QString)),
				this,
				SIGNAL (gotURL (QUrl, QString)));
	}

	QNetworkRequest Account::MakeRequest (const QUrl& url) const
	{
		QNetworkRequest rq (url);
		rq.setRawHeader ("Cache-Control", "no-cache");
		rq.setRawHeader ("Accept", "*/*");
		rq.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		return rq;
	}
}
}
}
