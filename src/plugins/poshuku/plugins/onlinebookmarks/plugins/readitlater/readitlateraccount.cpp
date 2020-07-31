/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "readitlateraccount.h"
#include <QDataStream>
#include <QtDebug>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <interfaces/ibookmarksservice.h>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterAccount::ReadItLaterAccount (const QString& login, QObject *parent)
	: QObject (parent)
	, Login_ (login)
	, ParentService_ (parent)
	, LastUpload_ (QDateTime::fromString ("01.01.1980", "ddMMyyyy"))
	, LastDownload_ (QDateTime::fromString ("01.01.1980", "ddMMyyyy"))
	{
	}

	QObject* ReadItLaterAccount::GetQObject ()
	{
		return this;
	}

	QObject* ReadItLaterAccount::GetParentService () const
	{
		return ParentService_;
	}

	QByteArray ReadItLaterAccount::GetAccountID () const
	{
		return QString ("org.LeechCraft.Poshuku.OnlineBookmarks.ReadItLater.%1")
				.arg (Login_).toUtf8 ();
	}

	QString ReadItLaterAccount::GetLogin () const
	{
		return Login_;
	}

	QString ReadItLaterAccount::GetPassword () const
	{
		return Password_;
	}

	void ReadItLaterAccount::SetPassword (const QString& pass)
	{
		Password_ = pass;
	}

	bool ReadItLaterAccount::IsSyncing () const
	{
		return IsSyncing_;
	}

	void ReadItLaterAccount::SetSyncing (bool sync)
	{
		IsSyncing_ = sync;
	}

	QDateTime ReadItLaterAccount::GetLastDownloadDateTime () const
	{
		return LastDownload_;
	}

	QDateTime ReadItLaterAccount::GetLastUploadDateTime () const
	{
		return LastUpload_;
	}

	void ReadItLaterAccount::SetLastDownloadDateTime(const QDateTime& date)
	{
		LastDownload_ = date;
	}

	void ReadItLaterAccount::AppendDownloadedBookmarks (const QVariantList& bookmarks)
	{
		for (const auto& var : bookmarks)
			if (!DownloadedBookmarks_.contains (var))
				DownloadedBookmarks_ << var;
	}

	QVariantList ReadItLaterAccount::GetBookmarksDiff (const QVariantList& list)
	{
		return Util::Filter (list,
				[this] (const auto& var) { return !DownloadedBookmarks_.contains (var); });
	}

	void ReadItLaterAccount::SetLastUploadDateTime(const QDateTime& date)
	{
		LastUpload_ = date;
	}

	QByteArray ReadItLaterAccount::Serialize () const
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

	ReadItLaterAccount* ReadItLaterAccount::Deserialize (const QByteArray& data,
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
		ReadItLaterAccount *acc = new ReadItLaterAccount (login, parent);
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
