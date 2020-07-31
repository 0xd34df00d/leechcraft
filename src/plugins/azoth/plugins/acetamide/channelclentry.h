/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCLENTRY_H

#include <QObject>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "localtypes.h"

namespace LC
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
		Q_INTERFACES (LC::Azoth::IMUCEntry
				LC::Azoth::ICLEntry
				LC::Azoth::IMUCPerms
				LC::Azoth::IConfigurableMUC)

		ChannelHandler *ICH_;
		QList<IMessage*> AllMessages_;
		bool IsWidgetRequest_;

		QMap<QByteArray, QList<QByteArray>> Perms_;
		QMap<ChannelRole, QByteArray> Role2Str_;
		QMap<ChannelRole, QByteArray> Aff2Str_;
		QMap<ChannelManagment, QByteArray> Managment2Str_;
		QMap<QByteArray, QString> Translations_;
	public:
		ChannelCLEntry (ChannelHandler*);
		ChannelHandler* GetChannelHandler () const;

		QObject* GetQObject ();
		IAccount* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();

		QString GetRealID (QObject*) const;

		EntryStatus GetStatus (const QString& variant = QString ()) const;
		void ShowInfo ();

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		bool CanChangeSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		bool IsAutojoined () const;
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
		QPair<QByteArray, QByteArray> GetKickPerm () const;
		QPair<QByteArray, QByteArray> GetBanPerm () const;
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
