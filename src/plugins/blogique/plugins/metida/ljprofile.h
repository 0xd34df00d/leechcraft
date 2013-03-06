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
#include <QSet>
#include <interfaces/blogique/iprofile.h>
#include "profiletypes.h"
#include "ljfriendentry.h"

class QNetworkReply;

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;
	class ProfileWidget;

	class LJProfile : public QObject
					, public IProfile
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IProfile)

		QObject *ParentAccount_;
		LJProfileData ProfileData_;
		QHash<QNetworkReply*, QString> Reply2AvatarId_;
	public:
		LJProfile (QObject *parentAccount, QObject *parent = 0);

		QWidget* GetProfileWidget ();
		QList<QPair<QIcon, QString>> GetPostingTargets () const;

		LJProfileData GetProfileData () const;
		QObject* GetParentAccount () const;

		void AddFriends (const QList<LJFriendEntry_ptr>& friends);
		QList<LJFriendEntry_ptr> GetFriends () const;

		QList<LJFriendGroup> GetFriendGroups () const;

		int GetFreeGroupId () const;

		void RequestInbox ();
	private:
		void SaveAvatar (QUrl url = QUrl ());
		void SaveOthersAvatars (const QString& id, const QUrl& url);

	public slots:
		void handleProfileUpdate (const LJProfileData& profile);
	private slots:
		void handleAvatarDownloadFinished ();
		void handleOtherAvatarDownloadFinished ();

	signals:
		void profileUpdated ();
	};
}
}
}
