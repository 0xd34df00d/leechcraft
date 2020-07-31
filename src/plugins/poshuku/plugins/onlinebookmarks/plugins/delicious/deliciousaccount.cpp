/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "deliciousaccount.h"
#include <QDataStream>
#include <QtDebug>
#include <QIcon>
#include <util/util.h>
#include <util/sll/prelude.h>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	DeliciousAccount::DeliciousAccount (const QString& login, QObject *parent)
	: QObject (parent)
	, Login_ (login)
	, ParentService_ (parent)
	, LastUpload_ (QDateTime::fromString ("01.01.1980", "ddMMyyyy"))
	, LastDownload_ (QDateTime::fromString ("01.01.1980", "ddMMyyyy"))
	{
	}

	QObject* DeliciousAccount::GetQObject ()
	{
		return this;
	}

	QObject* DeliciousAccount::GetParentService () const
	{
		return ParentService_;
	}

	QByteArray DeliciousAccount::GetAccountID () const
	{
		return QString ("org.LeechCraft.Poshuku.OnlineBookmarks.Delicious.%1")
				.arg (Login_).toUtf8 ();
	}

	QString DeliciousAccount::GetLogin () const
	{
		return Login_;
	}

	QString DeliciousAccount::GetPassword () const
	{
		return Password_;
	}

	void DeliciousAccount::SetPassword (const QString& pass)
	{
		Password_ = pass;
	}

	QVariantMap DeliciousAccount::GetIdentifyingData () const
	{
		return
		{
			{ "Login", Login_ },
			{ "Password_", Password_ }
		};
	}

	bool DeliciousAccount::IsSyncing () const
	{
		return IsSyncing_;
	}

	void DeliciousAccount::SetSyncing (bool sync)
	{
		IsSyncing_ = sync;
	}

	QDateTime DeliciousAccount::GetLastDownloadDateTime () const
	{
		return LastDownload_;
	}

	void DeliciousAccount::SetLastDownloadDateTime (const QDateTime& dt)
	{
		LastDownload_ = dt;
	}

	QDateTime DeliciousAccount::GetLastUploadDateTime () const
	{
		return LastUpload_;
	}

	void DeliciousAccount::SetLastUploadDateTime (const QDateTime& dt)
	{
		LastUpload_ = dt;
	}

	QVariantList DeliciousAccount::GetBookmarksDiff (const QVariantList& list)
	{
		return Util::Filter (list, [this] (const QVariant& var) { return !DownloadedBookmarks_.contains (var); });
	}

	void DeliciousAccount::AppendDownloadedBookmarks (const QVariantList& bookmarks)
	{
		for (const auto& var : bookmarks)
			if (!DownloadedBookmarks_.contains (var))
				DownloadedBookmarks_ << var;
	}

	QByteArray DeliciousAccount::Serialize () const
	{
		quint16 version = 1;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
					<< Login_
					<< IsSyncing_
					<< LastUpload_
					<< LastDownload_
					<< DownloadedBookmarks_;
		}

		return result;
	}

	DeliciousAccount* DeliciousAccount::Deserialize (const QByteArray& data,
			QObject *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString login;
		in >> login;
		DeliciousAccount *acc = new DeliciousAccount (login, parent);
		in >> acc->IsSyncing_
			>> acc->LastUpload_
			>> acc->LastDownload_
			>> acc->DownloadedBookmarks_;
		return acc;
	}

}
}
}
}
