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

#include "readitlaterapi.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterApi::ReadItLaterApi ()
	: ApiKey_ ("0l7A6m89daNpif742cpM7fRJe9Tcxd49")
	{
	}

	QString ReadItLaterApi::GetAuthUrl () const
	{
		return "https://readitlaterlist.com/v2/auth?";
	}

	QByteArray ReadItLaterApi::GetAuthPayload (const QString& login, const QString& pass)
	{
		return QString ("username=%1&password=%2&apikey=%3")
				.arg (login, pass, ApiKey_).toUtf8 ();
	}

	QString ReadItLaterApi::GetRegisterUrl () const
	{
		return "https://readitlaterlist.com/v2/signup?";
	}

	QByteArray ReadItLaterApi::GetRegisterPayload (const QString& login,
			const QString& pass)
	{
		return GetAuthPayload (login, pass);
	}

}
}
}
}

