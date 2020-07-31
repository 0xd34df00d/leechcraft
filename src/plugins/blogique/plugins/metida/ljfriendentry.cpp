/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ljfriendentry.h"
#include <QDataStream>
#include <QtDebug>

namespace LC
{
namespace Blogique
{
namespace Metida
{
	LJFriendEntry::LJFriendEntry (QObject *parent)
	: QObject (parent)
	, GroupMask_ (0)
	, FriendOf_ (false)
	, MyFriend_ (false)
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

	uint LJFriendEntry::GetGroupMask () const
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
		Birthday_ = date;
	}

	QString LJFriendEntry::GetBirthday () const
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

	void LJFriendEntry::SetMyFriend (bool myFriend)
	{
		MyFriend_ = myFriend;
	}

	bool LJFriendEntry::GetMyFriend () const
	{
		return MyFriend_;
	}

	QByteArray LJFriendEntry::Serialize () const
	{
		quint16 ver = 3;
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << ver
					<< UserName_
					<< FullName_
					<< AvatarUrl_
					<< BGColor_.name ()
					<< FGColor_.name ()
					<< GroupMask_
					<< Birthday_
					<< FriendOf_
					<< MyFriend_;
		}

		return result;
	}

	LJFriendEntry_ptr LJFriendEntry::Deserialize (const QByteArray& data)
	{
		quint16 ver;
		QDataStream in (data);
		in >> ver;

		if (ver < 1 ||
				ver > 3)
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

		if (ver >= 2)
			in >> result->Birthday_
					>> result->FriendOf_;

		if (ver == 3)
			in >> result->MyFriend_;

		return result;
	}

	bool LJFriendEntry::operator== (const LJFriendEntry& entry) const
	{
		return UserName_ == entry.UserName_;
	}
}
}
}
