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
	QDataStream& operator<< (QDataStream& out, const LJFriendGroup& group)
	{
		out << static_cast<qint8> (1)
				<< group.Public_
				<< group.Name_
				<< group.Id_
				<< group.SortOrder_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, LJFriendGroup& group)
	{
		qint8 version;
		in >> version;
		if (version == 1)
			in >> group.Public_
					>> group.Name_
					>> group.Id_
					>> group.SortOrder_;

		return in;
	}


	QDataStream& operator<< (QDataStream& out, const LJMood& mood)
	{
		out << static_cast<qint8> (1)
				<< mood.Id_
				<< mood.Name_
				<< mood.Parent_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, LJMood& mood)
	{
		qint8 version;
		in >> version;
		if (version == 1)
			in >> mood.Id_
					>> mood.Name_
					>> mood.Parent_;
		return in;
	}

	QDataStream& operator<< (QDataStream& out, const LJProfileData& data)
	{
		out << static_cast<qint8> (1)
				<< data.AvatarUrl_
				<< data.Caps_
				<< data.Communities_
				<< data.FullName_
				<< data.UserId_
				<< data.FriendGroups_
				<< data.Moods_;

		return out;
	}

	QDataStream& operator>> (QDataStream& in, LJProfileData& data)
	{
		qint8 version;
		in >> version;
		if (version == 1)
			in >> data.AvatarUrl_
					>> data.Caps_
					>> data.Communities_
					>> data.FullName_
					>> data.UserId_
					>> data.FriendGroups_
					>> data.Moods_;

		return in;
	}

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