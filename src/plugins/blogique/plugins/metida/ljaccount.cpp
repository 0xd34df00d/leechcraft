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

#include "ljaccount.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJAccount::LJAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, ParentBloggingPlatform_ (parent)
	, Name_ (name)
	{
	}

	QObject* LJAccount::GetObject ()
	{
		return this;
	}

	QObject* LJAccount::GetParentBloggingPlatform () const
	{
		return ParentBloggingPlatform_;
	}

	QString LJAccount::GetAccountName () const
	{
		return Name_;
	}

	QString LJAccount::GetOurLogin () const
	{
		//TODO
		return QString ();
	}

	void LJAccount::RenameAccount (const QString& name)
	{

	}

	QByteArray LJAccount::GetAccountID () const
	{
		//TODO
		return QByteArray ();
	}

	void LJAccount::OpenConfigurationDialog ()
	{

	}
}
}
}
