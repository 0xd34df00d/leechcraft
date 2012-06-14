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
		QMap<QString, QUrl> MenuList_;
		QList<LJFriendGroup> FriendGroups_;
		QList<LJMood> Moods_;
	};
}
}
}