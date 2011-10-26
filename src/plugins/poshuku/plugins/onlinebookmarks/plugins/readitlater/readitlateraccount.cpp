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

#include "readitlateraccount.h"
#include <QDataStream>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/ibookmarksservice.h>

namespace LeechCraft
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
	, IsSyncing_ (false)
	, IsQuickUpload_ (false)
	, LastUpload_ (QDateTime::currentDateTime ())
	, LastDownload_ (QDateTime::currentDateTime ())
	{
	}

	QObject* ReadItLaterAccount::GetObject ()
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

	bool ReadItLaterAccount::IsQuickUpload () const
	{
		return IsQuickUpload_;
	}

	void ReadItLaterAccount::SetQuickUpload (bool quick)
	{
		IsQuickUpload_ = quick;
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

	QVariantList ReadItLaterAccount::GetBookmarksDiff (const QVariantList& list)
	{
		QVariantList diff;
		Q_FOREACH (const QVariant& var, list)
		if (!DownloadedBookmarks_.contains (var))
			diff << var;

		return diff;
	}

	void ReadItLaterAccount::AppendDownloadedBookmarks (const QVariantList& bookmarks)
	{
		Q_FOREACH (const QVariant& var, bookmarks)
		if (!DownloadedBookmarks_.contains (var))
			DownloadedBookmarks_ << var;
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
					<< IsQuickUpload_
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
			>> acc->IsQuickUpload_
			>> acc->LastUpload_
			>> acc->LastDownload_
			>> acc->DownloadedBookmarks_;
		return acc;
	}


}
}
}
}
