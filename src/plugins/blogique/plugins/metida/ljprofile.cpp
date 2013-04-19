/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "ljprofile.h"
#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <util/util.h>
#include "ljaccount.h"
#include "core.h"
#include "profilewidget.h"
#include "localstorage.h"

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
		std::sort (ProfileData_.Friends_.begin (), ProfileData_.Friends_.end (),
				CompareFriends);
		ProfileData_.Friends_.erase (std::unique (ProfileData_.Friends_.begin (),
				ProfileData_.Friends_.end (),
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



