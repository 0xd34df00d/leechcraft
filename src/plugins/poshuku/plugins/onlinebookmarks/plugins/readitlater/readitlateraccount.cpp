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

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterAccount::ReadItLaterAccount (const QString& login,
			const QString& password, QObject *parent)
	: QObject (parent)
	, Login_ (login)
	, Password_ (password)
	, AuthType_ (IAccount::ATHttpAuth)
	, ParentService_ (parent)
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

	IAccount::AuthType ReadItLaterAccount::GetAuthType () const
	{
		return AuthType_;
	}

	QVariantMap ReadItLaterAccount::GetIdentifyingData () const
	{
		QVariantMap map;
		map ["Login"] = Login_;
		map ["Password_"] = Password_;
		return map;
	}

	QByteArray ReadItLaterAccount::Serialize () const
	{
		quint16 version = 1;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
					<< Login_
					<< Password_;
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
		QString password;
		in >> login
				>> password;
		return new ReadItLaterAccount (login, password, parent);
	}
}
}
}
}
