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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QStringList>
#include <QXmppMucIq.h>
#include <interfaces/iclentry.h>
#include <interfaces/imucentry.h>
#include <interfaces/imucperms.h>
#include <interfaces/iconfigurablemuc.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class RoomPublicMessage;
	class RoomHandler;

	class RoomCLEntry : public QObject
					  , public ICLEntry
					  , public IMUCEntry
					  , public IMUCPerms
					  , public IConfigurableMUC
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry
						LeechCraft::Azoth::IMUCEntry
						LeechCraft::Azoth::IMUCPerms
						LeechCraft::Azoth::IConfigurableMUC);

		friend class RoomHandler;

		GlooxAccount *Account_;
		QList<QObject*> AllMessages_;
		RoomHandler *RH_;
		QMap<QByteArray, QList<QByteArray> > Perms_;
		QMap<QXmppMucItem::Role, QByteArray> Role2Str_;
		QMap<QXmppMucItem::Affiliation, QByteArray> Aff2Str_;
		QMap<QByteArray, QString> Translations_;

		mutable QAction *ActionRequestVoice_;
	public:
		RoomCLEntry (RoomHandler*, GlooxAccount*);

		RoomHandler* GetRoomHandler () const;

		// ICLEntry
		QObject* GetObject ();
		QObject* GetParentAccount () const ;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		void Join ();
		void Leave (const QString&);
		QString GetNick () const;
		void SetNick (const QString&);
		QString GetGroupName () const;
		QVariantMap GetIdentifyingData () const;
		QString GetRealID (QObject*) const;
		void InviteToMUC (const QString&, const QString&);

		// IMUCPerms
		QMap<QByteArray, QList<QByteArray> > GetPossiblePerms () const;
		QMap<QByteArray, QList<QByteArray> > GetPerms (QObject *object) const;
		QByteArray GetAffName (QObject*) const;
		bool MayChangePerm (QObject*, const QByteArray&, const QByteArray&) const;
		void SetPerm (QObject*, const QByteArray&, const QByteArray&, const QString&);
		bool IsLessByPerm (QObject*, QObject*) const;
		bool IsMultiPerm (const QByteArray&) const;
		QString GetUserString (const QByteArray&) const;

		// IConfigurableMUC
		QWidget* GetConfigurationWidget ();
		void AcceptConfiguration (QWidget*);

		void HandleMessage (RoomPublicMessage*);
		void HandleNewParticipants (const QList<ICLEntry*>&);
		void HandleSubjectChanged (const QString&);
	private slots:
		void reemitStatusChange (const EntryStatus&);
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

		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);
		void nicknameConflict (const QString&);
		void beenKicked (const QString&);
		void beenBanned (const QString&);
	};
}
}
}

#endif
