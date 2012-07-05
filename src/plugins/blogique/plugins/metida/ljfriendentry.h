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

#include <memory>
#include <QObject>
#include <QUrl>
#include <QDateTime>
#include <QColor>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;
	typedef std::shared_ptr<LJFriendEntry> LJFriendEntry_ptr;

	class LJFriendEntry : public QObject
	{
		Q_OBJECT

		QUrl AvatarUrl_;
		QString FullName_;
		QString UserName_;
		uint GroupMask_;
		QColor BGColor_;
		QColor FGColor_;
		QString Birthday_;
		bool FriendOf_;
	public:
		LJFriendEntry (QObject *parent = 0);

		void SetAvatarUrl (const QUrl& url);
		QUrl GetAvatarurl () const;
		void SetFullName (const QString& fullName);
		QString GetFullName () const;
		void SetUserName (const QString& userName);
		QString GetUserName () const;
		void SetGroupMask (int groupmask);
		uint GetGroupMask () const;
		void SetBGColor (const QString& name);
		QColor GetBGColor () const;
		void SetFGColor (const QString& name);
		QColor GetFGColor () const;
		void SetBirthday (const QString& date);
		QString GetBirthday () const;
		void SetFriendOf (bool friendOf);
		bool GetFriendOf () const;

		QByteArray Serialize () const;
		static LJFriendEntry_ptr Deserialize (const QByteArray& data);
	};
}
}
}
