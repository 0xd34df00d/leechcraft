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

#pragma once

#include <QUrl>
#include <QMap>
#include <QStringList>
#include <QVariant>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	struct LJFriendGroup
	{
		bool Public_;
		QString Name_;
		uint Id_;
		uint SortOrder_;
	};

	struct LJMood
	{
		qint64 Parent_;
		qint64 Id_;
		QString Name_;
	};

	struct LJProfileData
	{
		QUrl AvatarUrl_;
		qint64 UserId_;
		qint64 Caps_;
		QList<LJFriendGroup> FriendGroups_;
		QList<LJMood> Moods_;
		QStringList Communities_;
		QString FullName_;
	};

	namespace LJParserTypes
	{
		class LJParseProfileEntry
		{
			QString Name_;
			QVariantList ValueList_;

		public:
			LJParseProfileEntry ();
			LJParseProfileEntry (const QString& name,
					const QVariantList& value);
			QString Name () const;
			QVariantList Value () const;

			bool ValueToBool () const;
			QString ValueToString () const;
			qint64 ValueToLongLong () const;
			int ValueToInt () const;
			QUrl ValueToUrl () const;
		};
	}

	QDataStream& operator<< (QDataStream& out, const LJFriendGroup& group);
	QDataStream& operator>> (QDataStream& in, LJFriendGroup& group);
	QDataStream& operator<< (QDataStream& out, const LJMood& mood);
	QDataStream& operator>> (QDataStream& in, LJMood& mood);
	QDataStream& operator<< (QDataStream& out, const LJProfileData& data);
	QDataStream& operator>> (QDataStream& in, LJProfileData& data);
}
}
}

Q_DECLARE_METATYPE (LeechCraft::Blogique::Metida::LJParserTypes::LJParseProfileEntry)
Q_DECLARE_METATYPE (LeechCraft::Blogique::Metida::LJProfileData)