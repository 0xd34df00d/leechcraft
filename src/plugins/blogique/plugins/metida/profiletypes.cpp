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
#include <QtDebug>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	QDataStream& operator<< (QDataStream& out, const LJFriendGroup& group)
	{
		out << static_cast<qint8> (2)
				<< group.Public_
				<< group.Name_
				<< group.Id_
				<< group.SortOrder_
				<< group.RealId_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, LJFriendGroup& group)
	{
		qint8 version = 0;
		in >> version;
		if (version > 0)
			in >> group.Public_
					>> group.Name_
					>> group.Id_
					>> group.SortOrder_;
		if (version == 2)
			in >> group.RealId_;

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
		qint8 version = 0;
		in >> version;
		if (version == 1)
			in >> mood.Id_
					>> mood.Name_
					>> mood.Parent_;
		return in;
	}

	QDataStream& operator<< (QDataStream& out, const LJProfileData& data)
	{
		out << static_cast<qint8> (3)
				<< data.AvatarUrl_
				<< data.Caps_
				<< data.Communities_
				<< data.FullName_
				<< data.UserId_
				<< data.FriendGroups_
				<< data.Moods_
				<< data.Friends_.count ();

		for (const auto& fr : data.Friends_)
		{
			QByteArray ba = fr->Serialize ();
			out << ba;
		}

		out << data.AvatarsID_
				<< data.AvatarsUrls_;

		return out;
	}

	QDataStream& operator>> (QDataStream& in, LJProfileData& data)
	{
		qint8 version = 0;
		in >> version;
		if (version > 0)
			in >> data.AvatarUrl_
					>> data.Caps_
					>> data.Communities_
					>> data.FullName_
					>> data.UserId_
					>> data.FriendGroups_
					>> data.Moods_;
		if (version >= 2)
		{
			int count = 0;
			in >> count;
			for (int i = 0; i < count; ++i)
			{
				QByteArray ba;
				in >> ba;
				data.Friends_ << LJFriendEntry::Deserialize (ba);
			}

			data.Friends_.removeAll (0);
		}

		if (version >= 3)
		{
			in >> data.AvatarsID_
					>> data.AvatarsUrls_;
		}
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
