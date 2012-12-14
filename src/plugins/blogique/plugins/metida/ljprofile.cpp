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

#include "ljprofile.h"
#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <util/util.h>
#include <interfaces/blogique/iaccount.h>
#include "core.h"
#include "profilewidget.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJProfile::LJProfile (QObject *parentAccount, QObject *parent)
	: QObject (parent)
	, ParentAccount_ (parentAccount)
	{
	}

	QWidget* LJProfile::GetProfileWidget ()
	{
		return new ProfileWidget (this);
	}

	QList<QPair<QIcon, QString>> LJProfile::GetPostingTargets () const
	{
		QList<QPair<QIcon, QString>> targets;
		const QIcon& icon = Core::Instance ().GetCoreProxy ()->GetIcon ("system-users");
		IAccount *acc = qobject_cast<IAccount*> (ParentAccount_);
		if (!acc)
			return targets;

		targets.append ({ Core::Instance ().GetCoreProxy ()->GetIcon ("im-user"),
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
		ProfileData_.Friends_ << friends;
		std::sort (ProfileData_.Friends_.begin (), ProfileData_.Friends_.end (), CompareFriends);
		ProfileData_.Friends_.erase (std::unique (ProfileData_.Friends_.begin (), ProfileData_.Friends_.end (),
				[] (decltype (ProfileData_.Friends_.front ()) fr1,
						decltype (ProfileData_.Friends_.front ()) fr2)
				{
					return fr1->GetUserName () == fr2->GetUserName ();
				}), ProfileData_.Friends_.end ());

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
		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->get (request);
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
		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->get (request);
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
				[] (decltype (ProfileData_.Moods_.at (0)) mood1,
						decltype (ProfileData_.Moods_.at (0)) mood2)
					{ return QString::localeAwareCompare (mood1.Name_, mood2.Name_) < 0; });
		SaveAvatar ();
		for (int i = 0; i < ProfileData_.AvatarsID_.count (); ++i)
			SaveOthersAvatars (ProfileData_.AvatarsID_.value (i),
					ProfileData_.AvatarsUrls_.value (i));
		emit profileUpdated ();
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
		const QDir& avatarDir = Util::CreateIfNotExists ("blogique/metida/avatars");

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
		const QDir& avatarDir = Util::CreateIfNotExists ("blogique/metida/avatars");

		const QString& path = avatarDir.absoluteFilePath (filename);
		QFile file (path);
		if (file.open (QIODevice::WriteOnly))
			file.write (reply->readAll ());
	}

}
}
}



