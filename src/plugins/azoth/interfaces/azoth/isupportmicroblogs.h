/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include <QString>
#include <QList>
#include <QMap>
#include <QDateTime>

namespace LeechCraft
{
namespace Azoth
{
	struct PostAuthor
	{
		QString Name_;
		QString URI_;
	};

	struct Post
	{
		QString ID_;
		QMap<QString, QString> Contents_;

		QDateTime Published_;
		QDateTime Updated_;

		PostAuthor Author_;
	};

	class ISupportMicroblogs
	{
	public:
		virtual ~ISupportMicroblogs () {}

		virtual void RequestLastPosts (int) = 0;
	protected:
		virtual void gotNewPost (const Post&) = 0;

		virtual void gotRecentPosts (const QList<Post>&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportMicroblogs,
		"org.Deviant.LeechCraft.Azoth.ISupportMicroblogs/1.0");
