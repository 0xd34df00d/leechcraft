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
#include <QSet>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/blogique/iprofile.h>
#include "profiletypes.h"
#include "ljfriendentry.h"

class QNetworkReply;

namespace LC
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
		Q_INTERFACES (LC::Blogique::IProfile)

		QObject * const ParentAccount_;
		const ICoreProxy_ptr Proxy_;

		LJProfileData ProfileData_;
		QHash<QNetworkReply*, QString> Reply2AvatarId_;
	public:
		LJProfile (QObject *parentAccount, const ICoreProxy_ptr&, QObject *parent = nullptr);

		QWidget* GetProfileWidget () override;
		QList<QPair<QIcon, QString>> GetPostingTargets () const override;

		LJProfileData GetProfileData () const;
		QObject* GetParentAccount () const;

		void AddFriends (const QList<LJFriendEntry_ptr>& friends);
		QList<LJFriendEntry_ptr> GetFriends () const;

		QList<LJFriendGroup> GetFriendGroups () const;

		QHash<QString, int> GetTags () const;

		int GetFreeGroupId () const;

	private:
		void SaveAvatar (QUrl url = QUrl ());
		void SaveOthersAvatars (const QString& id, const QUrl& url);

	public slots:
		void handleProfileUpdate (const LJProfileData& profile);
		void handleGotTags (const QHash<QString, int>& tags);
	private slots:
		void handleAvatarDownloadFinished ();
		void handleOtherAvatarDownloadFinished ();

	signals:
		void profileUpdated () override;
		void tagsUpdated (const QHash<QString, int>& tags);
	};
}
}
}
