/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_VADER_MRIMBUDDY_H
#define PLUGINS_AZOTH_PLUGINS_VADER_MRIMBUDDY_H
#include <QObject>
#include <interfaces/iclentry.h>
#include <interfaces/iadvancedclentry.h>
#include "proto/contactinfo.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	class MRIMAccount;
	class MRIMMessage;

	class MRIMBuddy : public QObject
					, public ICLEntry
					, public IAdvancedCLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry
				LeechCraft::Azoth::IAdvancedCLEntry);

		MRIMAccount *A_;
		Proto::ContactInfo Info_;
		QString Group_;

		EntryStatus Status_;

		QList<MRIMMessage*> AllMessages_;

		bool IsAuthorized_;
		
		QVariantMap ClientInfo_; 
	public:
		MRIMBuddy (const Proto::ContactInfo&, MRIMAccount*);

		void HandleMessage (MRIMMessage*);
		void HandleAttention (const QString&);
		void HandleTune (const QString&);
		void SetGroup (const QString&);

		void SetAuthorized (bool);
		bool IsAuthorized () const;

		Proto::ContactInfo GetInfo () const;
		void UpdateInfo (const Proto::ContactInfo&);

		qint64 GetID () const;

		// ICLEntry
		QObject* GetObject ();
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString& name);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList& groups);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType, const QString&, const QString&);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();

		// IAdvancedCLEntry
		AdvancedFeatures GetAdvancedFeatures () const;
		void DrawAttention (const QString&, const QString&);
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

		void attentionDrawn (const QString&, const QString&);
		void moodChanged (const QString&);
		void activityChanged (const QString&);
		void tuneChanged (const QString&);
		void locationChanged (const QString&);
	};
}
}
}

#endif
