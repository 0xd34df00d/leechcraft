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
#include <QtDebug>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJFriendEntry::LJFriendEntry (QObject *parent)
	: QObject (parent)
	, GroupMask_ (0)
	, FriendOf_ (false)
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

	void LJFriendEntry::SetBirthday (const QString& date)
	{
		//TODO
		Birthday_ = QDateTime::fromString (date, "");
	}

	QDateTime LJFriendEntry::GetBirthday () const
	{
		return Birthday_;
	}

	void LJFriendEntry::SetFriendOf (bool friendOf)
	{
		FriendOf_ = friendOf;
	}

	bool LJFriendEntry::GetFriendOf () const
	{
		return FriendOf_;
	}

	QByteArray LJFriendEntry::Serialize () const
	{
		quint16 ver = 1;
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << ver
					<< UserName_
					<< FullName_
					<< AvatarUrl_
					<< BGColor_.name ()
					<< FGColor_.name ()
					<< GroupMask_;
		}

		return result;
	}

	LJFriendEntry_ptr LJFriendEntry::Deserialize (const QByteArray& data)
	{
		quint16 ver;
		QDataStream in (data);
		in >> ver;

		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return 0;
		}

		LJFriendEntry_ptr result = std::make_shared<LJFriendEntry> ();
		QString bgColorName, fgColorName;
		in >> result->UserName_
				>> result->FullName_
				>> result->AvatarUrl_
				>> bgColorName
				>> fgColorName
				>> result->GroupMask_;
		result->BGColor_.setNamedColor (bgColorName);
		result->FGColor_.setNamedColor (fgColorName);

		return result;
	}

}
}
}
