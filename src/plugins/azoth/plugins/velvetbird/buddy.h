/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <blist.h>
#include <conversation.h>
#include <interfaces/azoth/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	class Account;
	class ConvIMMessage;

	class Buddy : public QObject
				, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry)

		Account *Account_;
		PurpleBuddy *Buddy_;

		QString Name_;
		EntryStatus Status_;

		QString Group_;

		QList<ConvIMMessage*> Messages_;

		std::shared_ptr<PurpleConversation> PurpleConv_;
	public:
		Buddy (PurpleBuddy*, Account*);

		QObject* GetObject ();
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList& groups);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType type, const QString& variant, const QString& body);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime& before);
		void SetChatPartState (ChatPartState state, const QString& variant);
		EntryStatus GetStatus (const QString& variant = QString ()) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString& variant) const;
		void MarkMsgsRead ();

		void Send (ConvIMMessage*);
		void Store (ConvIMMessage*);
		void SetConv (PurpleConversation*);
		void HandleMessage (const char*, const char*, PurpleMessageFlags, time_t);

		PurpleBuddy* GetPurpleBuddy () const;
		void Update ();
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();
	};
}
}
}
