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

#include "profiletypes.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
namespace LJParserTypes
{
	LJParseProfileEntry::LJParseProfileEntry ()
	{
	}

	LJParseProfileEntry::LJParseProfileEntry (const QString& name,
			const QVariantList& value)
	: Name_ (name)
	, ValueList_ (value)
	{
	}

	QString LJParseProfileEntry::Name () const
	{
		return Name_;
	}

	QVariantList LJParseProfileEntry::Value () const
	{
		return ValueList_;
	}

	bool LJParseProfileEntry::ValueToBool () const
	{
		return ValueList_.value (0).toBool ();
	}

	QString LJParseProfileEntry::ValueToString () const
	{
		return ValueList_.value (0).toString ();
	}

	qint64 LJParseProfileEntry::ValueToLongLong () const
	{
		return ValueList_.value (0).toLongLong ();
	}

	int LJParseProfileEntry::ValueToInt () const
	{
		return ValueList_.value (0).toInt ();
	}

	QUrl LJParseProfileEntry::ValueToUrl () const
	{
		return ValueList_.value (0).toUrl ();
	}
}
}
}
}