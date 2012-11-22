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
#include <interfaces/blogique/iaccount.h>
#include "profiletypes.h"
#include "ljfriendentry.h"
#include "entryoptions.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	struct LJEventProperties
	{
		QString CurrentLocation_;
		QString CurrentMood_;
		int CurrentMoodId_;
		QString CurrentMusic_;
		bool ShowInFriendsPage_;
		bool AutoFormat_;
		AdultContent AdultContent_;
		CommentsManagement CommentsManagement_;
		CommentsManagement ScreeningComments_;
		QString PostAvatar_;
		bool EntryVisibility_;
		bool UsedRTE_;
	};

	struct LJEvent
	{
		QString Event_;
		QString Subject_;
		Access Security_;
		quint32 AllowMask_;
		QDateTime DateTime_;
		QStringList Tags_;
		QString TimeZone_;
		QString UseJournal_;
		LJEventProperties Props_;
	};

	class LJFriendEntry;
	class LJAccountConfigurationWidget;
	class LJBloggingPlatform;
	class LJXmlRPC;
	class LJProfile;

	class LJAccount : public QObject
					, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IAccount)

		LJBloggingPlatform *ParentBloggingPlatform_;
		LJXmlRPC *LJXmlRpc_;
		QString Name_;
		QString Login_;
		bool IsValidated_;
		std::shared_ptr<LJProfile> LJProfile_;
	public:
		LJAccount (const QString& name, QObject *parent = 0);

		QObject* GetObject ();
		QObject* GetParentBloggingPlatform () const;
		QString GetAccountName () const;
		QString GetOurLogin () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		void OpenConfigurationDialog ();
		bool IsValidated () const;

		QString GetPassword () const;

		QObject* GetProfile ();

		void FillSettings (LJAccountConfigurationWidget *widget);

		QByteArray Serialize () const;
		static LJAccount* Deserialize (const QByteArray& data, QObject *parent);

		void Validate ();
		void Init ();

		void AddFriends (const QList<LJFriendEntry_ptr>& friends);

		void AddNewFriend (const QString& username,
				const QString& bgcolor, const QString& fgcolor, uint groupId);
		void DeleteFriend (const QString& username);

		void AddGroup (const QString& name, bool isPublic, int id);
		void DeleteGroup (int id);

	public slots:
		void handleValidatingFinished (bool success);
		void handleXmlRpcError (int errorCode, const QString& msgInEng);
		void updateProfile ();
		void submit (const Event& event);
	signals:
		void accountRenamed (const QString& newName);
		void accountSettingsChanged ();
		void accountValidated (bool validated);
	};
}
}
}
