/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ljprofile.h"
#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoreproxy.h>
#include <util/sys/paths.h>
#include "ljaccount.h"
#include "profilewidget.h"
#include "localstorage.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	LJProfile::LJProfile (QObject *parentAccount, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, ParentAccount_ { parentAccount }
	, Proxy_ { proxy }
	{
	}

	QWidget* LJProfile::GetProfileWidget ()
	{
		return new ProfileWidget (this, Proxy_);
	}

	QList<QPair<QIcon, QString>> LJProfile::GetPostingTargets () const
	{
		const auto mgr = Proxy_->GetIconThemeManager ();

		QList<QPair<QIcon, QString>> targets;
		const auto& icon = mgr->GetIcon ("system-users");
		IAccount *acc = qobject_cast<IAccount*> (ParentAccount_);
		if (!acc)
			return targets;

		targets.append ({ mgr->GetIcon ("im-user"),
				acc->GetOurLogin () });
		for (const auto& community : ProfileData_.Communities_)
			targets.append ({ icon, community });
		return targets;
	}

	LJProfileData LJProfile::GetProfileData () const
	{
		return ProfileData_;
	}

	QObject* LJProfile::GetParentAccount () const
	{
		return ParentAccount_;
	}

	namespace
	{
		bool CompareFriends (const LJFriendEntry_ptr& fr1, const LJFriendEntry_ptr& fr2)
		{
			return fr1->GetUserName () < fr2->GetUserName ();
		}
	}

	void LJProfile::AddFriends (const QList<LJFriendEntry_ptr>& friends)
	{
		for (const auto& friendEntry : friends)
		{
			const int index = ProfileData_.Friends_.indexOf (friendEntry);
			if (index == -1)
				ProfileData_.Friends_ << friendEntry;
			else
				ProfileData_.Friends_.replace (index, friendEntry);
		}

		std::sort (ProfileData_.Friends_.begin (), ProfileData_.Friends_.end (), CompareFriends);

		handleProfileUpdate (ProfileData_);
		emit profileUpdated ();
	}

	QList<LJFriendEntry_ptr> LJProfile::GetFriends () const
	{
		return ProfileData_.Friends_;
	}

	QList<LJFriendGroup> LJProfile::GetFriendGroups () const
	{
		return ProfileData_.FriendGroups_;
	}

	QHash<QString, int> LJProfile::GetTags () const
	{
		return ProfileData_.Tags_;
	}

	int LJProfile::GetFreeGroupId () const
	{
		QVector<int> baseVector (30);
		int current = 0;
		std::generate (baseVector.begin (), baseVector.end (),
				[&current] () { return ++current; });

		QVector<int> existingIds;
		for (const auto& group : ProfileData_.FriendGroups_)
			existingIds.append (group.Id_);

		std::sort (existingIds.begin (), existingIds.end ());
		QVector<int> result;
		std::set_difference (baseVector.begin (), baseVector.end (),
				existingIds.begin (), existingIds.end (),
				std::back_inserter (result));

		return result.value (0, -1);
	}

	void LJProfile::SaveAvatar (QUrl avatarUrl)
	{
		if (avatarUrl.isEmpty ())
			avatarUrl = ProfileData_.AvatarUrl_;
		if (avatarUrl.isEmpty ())
			return;

		QNetworkRequest request (avatarUrl);
		const auto reply = Proxy_->GetNetworkAccessManager ()->get (request);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAvatarDownloadFinished ()));
	}

	void LJProfile::SaveOthersAvatars (const QString& id, const QUrl& url)
	{
		if (id.isEmpty () ||
				url.isEmpty ())
			return;

		QNetworkRequest request (url);
		const auto reply = Proxy_->GetNetworkAccessManager ()->get (request);
		Reply2AvatarId_ [reply] = id;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleOtherAvatarDownloadFinished ()));
	}

	void LJProfile::handleProfileUpdate (const LJProfileData& profile)
	{
		ProfileData_ = profile;
		std::sort (ProfileData_.Moods_.begin (), ProfileData_.Moods_.end (),
				[] (const auto& mood1, const auto& mood2)
					{ return QString::localeAwareCompare (mood1.Name_, mood2.Name_) < 0; });
		SaveAvatar ();
		for (int i = 0; i < ProfileData_.AvatarsID_.count (); ++i)
			SaveOthersAvatars (ProfileData_.AvatarsID_.value (i),
					ProfileData_.AvatarsUrls_.value (i));
	}

	void LJProfile::handleGotTags (const QHash<QString, int>& tags)
	{
		ProfileData_.Tags_ = tags;
		emit tagsUpdated (tags);
	}

	void LJProfile::handleAvatarDownloadFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		IAccount *acc = qobject_cast<IAccount*> (ParentAccount_);
		if (!acc)
			return;
		const QByteArray filename = acc->GetAccountID ().toBase64 ().replace ('/', '_');
		const QDir& avatarDir = Util::GetUserDir (Util::UserDir::Cache, "blogique/metida/avatars");

		const QString& path = avatarDir.absoluteFilePath (filename);
		QFile file (path);
		if (file.open (QIODevice::WriteOnly))
			file.write (reply->readAll ());
	}

	void LJProfile::handleOtherAvatarDownloadFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QString id = Reply2AvatarId_.take (reply);
		reply->deleteLater ();

		IAccount *acc = qobject_cast<IAccount*> (ParentAccount_);
		if (!acc)
			return;

		const QByteArray filename = (acc->GetAccountID () + id.toUtf8 ())
				.toBase64 ().replace ('/', '_');
		const QDir& avatarDir = Util::GetUserDir (Util::UserDir::Cache, "blogique/metida/avatars");

		const QString& path = avatarDir.absoluteFilePath (filename);
		QFile file (path);
		if (file.open (QIODevice::WriteOnly))
			file.write (reply->readAll ());
	}

}
}
}



