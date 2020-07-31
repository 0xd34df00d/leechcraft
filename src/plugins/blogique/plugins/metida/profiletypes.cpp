/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "profiletypes.h"
#include <QDataStream>
#include <QtDebug>

namespace LC
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
		out << static_cast<qint8> (4)
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
				<< data.AvatarsUrls_
				<< data.Tags_;

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

		if (version >= 4)
			in >> data.Tags_;

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
