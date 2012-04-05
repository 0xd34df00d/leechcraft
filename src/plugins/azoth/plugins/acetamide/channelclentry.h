/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H

#include <QObject>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class ChannelHandler;
	class ChannelPublicMessage;
	class ChannelParticipantEntry;

	class ChannelCLEntry : public QObject
						 , public ICLEntry
						 , public IMUCEntry
						 , public IMUCPerms
						 , public IConfigurableMUC

	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMUCEntry
				LeechCraft::Azoth::ICLEntry
				LeechCraft::Azoth::IMUCPerms
				LeechCraft::Azoth::IConfigurableMUC)

		ChannelHandler *ICH_;
		QList<QObject*> AllMessages_;
		bool IsWidgetRequest_;

		QMap<QByteArray, QList<QByteArray>> Perms_;
		QMap<ChannelRole, QByteArray> Role2Str_;
		QMap<ChannelRole, QByteArray> Aff2Str_;
		QMap<ChannelManagment, QByteArray> Managment2Str_;
		QMap<QByteArray, QString> Translations_;
	public:
		ChannelCLEntry (ChannelHandler*);
		ChannelHandler* GetChannelHandler () const;

		QObject* GetObject ();
		QObject* GetParentAccount () const;
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
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();

		QString GetRealID (QObject*) const;

		EntryStatus GetStatus (const QString& variant = QString ()) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();

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
		void InviteToMUC (const QString&, const QString&);

		void HandleMessage (ChannelPublicMessage*);
		void HandleNewParticipants (const QList<ICLEntry*>&);
		void HandleSubjectChanged (const QString&);

		// IMUCPerms
		QByteArray GetAffName (QObject*) const;
		QMap<QByteArray, QList<QByteArray>> GetPerms (QObject*) const;
		void SetPerm (QObject*, const QByteArray&, const QByteArray&,
					  const QString&);
		QMap<QByteArray, QList<QByteArray>> GetPossiblePerms () const;
		QString GetUserString (const QByteArray&) const;
		bool IsLessByPerm (QObject*, QObject*) const;
		bool MayChangePerm (QObject*, const QByteArray&,
							const QByteArray&) const;
		bool IsMultiPerm (const QByteArray&) const;

		ChannelModes GetChannelModes () const;

		// IConfigurableMUC
		QWidget* GetConfigurationWidget ();
		void AcceptConfiguration (QWidget*);
		void RequestBanList ();
		void RequestExceptList ();
		void RequestInviteList ();
		void SetBanListItem (const QString&, const QString&,
				const QDateTime&);
		void SetExceptListItem (const QString&, const QString&,
				const QDateTime&);
		void SetInviteListItem (const QString&, const QString&,
				const QDateTime&);
		void SetIsWidgetRequest (bool);
		bool GetIsWidgetRequest () const;
		void AddBanListItem (QString);
		void RemoveBanListItem (QString);
		void AddExceptListItem (QString);
		void RemoveExceptListItem (QString);
		void AddInviteListItem (QString);
		void RemoveInviteListItem (QString);
		void SetNewChannelModes (const ChannelModes&);
		QString Role2String (const ChannelRole&) const;
	signals:
		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);

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

		void nicknameConflict (const QString&);
		void beenKicked (const QString&);
		void beenBanned (const QString&);

		void gotBanListItem (const QString&,
				const QString&, const QDateTime&);
		void gotExceptListItem (const QString&,
				const QString&, const QDateTime&);
		void gotInviteListItem (const QString&,
				const QString&, const QDateTime&);
		void gotNewChannelModes (const ChannelModes&);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H
