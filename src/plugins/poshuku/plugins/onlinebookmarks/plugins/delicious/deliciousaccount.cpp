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

#include "deliciousaccount.h"
#include <QDataStream>
#include <QtDebug>
#include <util/util.h>

namespace LeechCraft
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
	, IsSyncing_ (false)
	, IsQuickUpload_ (false)
	, LastUpload_ (QDateTime::currentDateTime ())
	, LastDownload_ (QDateTime::currentDateTime ())
	{
	}

	QObject* DeliciousAccount::GetObject ()
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
		QVariantMap map;
		map ["Login"] = Login_;
		map ["Password_"] = Password_;
		return map;
	}

	bool DeliciousAccount::IsSyncing () const
	{
		return IsSyncing_;
	}

	void DeliciousAccount::SetSyncing (bool sync)
	{
		IsSyncing_ = sync;
	}

	bool DeliciousAccount::IsQuickUpload () const
	{
		return IsQuickUpload_;
	}

	void DeliciousAccount::SetQuickUpload (bool quick)
	{
		IsQuickUpload_ = quick;
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
		QVariantList diff;
		Q_FOREACH (const QVariant& var, list)
			if (!DownloadedBookmarks_.contains (var))
				diff << var;

		return diff;
	}

	void DeliciousAccount::AppendDownloadedBookmarks (const QVariantList& bookmarks)
	{
		Q_FOREACH (const QVariant& var, bookmarks)
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
					<< IsQuickUpload_
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
