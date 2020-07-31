/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QUrl>
#include <QDateTime>
#include <QColor>

namespace LC
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
		bool MyFriend_;
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
		void SetMyFriend (bool myFriend);
		bool GetMyFriend () const;

		QByteArray Serialize () const;
		static LJFriendEntry_ptr Deserialize (const QByteArray& data);
		
		bool operator== (const LJFriendEntry& entry) const;
	};
}
}
}
