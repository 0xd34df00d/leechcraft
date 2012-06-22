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

	LJProfileData LJProfile::GetProfileData () const
	{
		return ProfileData_;
	}

	QObject* LJProfile::GetParentAccount () const
	{
		return ParentAccount_;
	}

	void LJProfile::SaveAvatar (QUrl avatarUrl)
	{
		if (avatarUrl.isEmpty () &&
				ProfileData_.AvatarUrl_.isEmpty ())
			return;

		QUrl url = avatarUrl;
		if (url.isEmpty ())
			url = ProfileData_.AvatarUrl_;

		QNetworkRequest request (url);
		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->get (request);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAvatarDownloadFinished ()));
	}

	void LJProfile::handleProfileUpdate (const LJProfileData& profile)
	{
		ProfileData_ = profile;
		SaveAvatar ();
	}

	void LJProfile::handleAvatarDownloadFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		IAccount *acc = qobject_cast<LeechCraft::Blogique::IAccount*> (ParentAccount_);
		if (!acc)
			return;
		const QByteArray& filename = acc->GetAccountID ().toBase64 ();
		const QDir& avatarDir = Util::CreateIfNotExists ("blogique/metida/avatars");

		const QString& path = avatarDir.absoluteFilePath (filename);
		QFile file (path);
		if (file.open (QIODevice::WriteOnly))
			file.write (reply->readAll ());
	}

}
}
}



