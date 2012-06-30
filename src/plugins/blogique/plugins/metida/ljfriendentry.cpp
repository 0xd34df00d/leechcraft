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

#include "ljfriendentry.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJFriendEntry::LJFriendEntry (QObject *parent)
	: QObject (parent)
	{
	}

	void LJFriendEntry::SetAvatarUrl (const QUrl& url)
	{
		AvatarUrl_ = url;
	}

	QUrl LJFriendEntry::GetAvatarurl () const
	{
		return AvatarUrl_;
	}

	void LJFriendEntry::SetFullName (const QString& fullName)
	{
		FullName_ = fullName;
	}

	QString LJFriendEntry::GetFullName () const
	{
		return FullName_;
	}

	void LJFriendEntry::SetUserName (const QString& userName)
	{
		UserName_ = userName;
	}

	QString LJFriendEntry::GetUserName () const
	{
		return UserName_;
	}

	void LJFriendEntry::SetGroupMask (int groupmask)
	{
		GroupMask_ = groupmask;
	}

	int LJFriendEntry::GetGroupMask () const
	{
		return GroupMask_;
	}

	void LJFriendEntry::SetBGColor (const QString& name)
	{
		BGColor_.setNamedColor (name);
	}

	QColor LJFriendEntry::GetBGColor () const
	{
		return BGColor_;
	}

	void LJFriendEntry::SetFGColor (const QString& name)
	{
		FGColor_.setNamedColor (name);
	}

	QColor LJFriendEntry::GetFGColor () const
	{
		return FGColor_;
	}


	uint qHash (const std::shared_ptr<LJFriendEntry>& fr)
	{
		return qHash (fr->GetFullName () + fr->GetUserName ());
	}
}
}
}
