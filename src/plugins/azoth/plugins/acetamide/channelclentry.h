/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "localtypes.h"

namespace LC::Azoth::Acetamide
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

		ChannelHandler * const ICH_;
		QList<IMessage*> AllMessages_;
		bool IsWidgetRequest_ = false;

		QMap<QByteArray, QList<QByteArray>> Perms_;
		QMap<ChannelRole, QByteArray> Role2Str_;
		QMap<ChannelRole, QByteArray> Aff2Str_;
		QMap<ChannelManagment, QByteArray> Managment2Str_;
		QMap<QByteArray, QString> Translations_;
	public:
		explicit ChannelCLEntry (ChannelHandler*);

		ChannelHandler* GetChannelHandler () const;

		QObject* GetQObject () override;
		IAccount* GetParentAccount () const override;
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;
		QString GetEntryName () const override;
		void SetEntryName (const QString&) override;
		QString GetEntryID () const override;
		QString GetHumanReadableID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList&) override;
		QStringList Variants () const override;
		IMessage* CreateMessage (IMessage::Type, const QString&, const QString&) override;
		QList<IMessage*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime&) override;
		void SetChatPartState (ChatPartState, const QString&) override;
		QList<QAction*> GetActions () const override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;
		void MarkMsgsRead () override;
		void ChatTabClosed () override;

		QString GetRealID (QObject*) const override;

		EntryStatus GetStatus (const QString& variant = QString ()) const override;
		void ShowInfo () override;

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const override;
		QString GetMUCSubject () const override;
		bool CanChangeSubject () const override;
		void SetMUCSubject (const QString&) override;
		QList<QObject*> GetParticipants () override;
		bool IsAutojoined () const override;
		void Join () override;
		void Leave (const QString&) override;
		QString GetNick () const override;
		void SetNick (const QString&) override;
		QString GetGroupName () const override;
		QVariantMap GetIdentifyingData () const override;
		void InviteToMUC (const QString&, const QString&) override;

		void HandleMessage (ChannelPublicMessage*);
		void HandleNewParticipants (const QList<ICLEntry*>&);
		void HandleSubjectChanged (const QString&);

		// IMUCPerms
		QByteArray GetAffName (QObject*) const override;
		QMap<QByteArray, QList<QByteArray>> GetPerms (QObject*) const override;
		QPair<QByteArray, QByteArray> GetKickPerm () const override;
		QPair<QByteArray, QByteArray> GetBanPerm () const override;
		void SetPerm (QObject*, const QByteArray&, const QByteArray&, const QString&) override;
		QMap<QByteArray, QList<QByteArray>> GetPossiblePerms () const override;
		QString GetUserString (const QByteArray&) const override;
		bool IsLessByPerm (QObject*, QObject*) const override;
		bool MayChangePerm (QObject*, const QByteArray&, const QByteArray&) const override;
		bool IsMultiPerm (const QByteArray&) const override;

		ChannelModes GetChannelModes () const;

		// IConfigurableMUC
		QWidget* GetConfigurationWidget () override;
		void AcceptConfiguration (QWidget*) override;

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
		void gotNewParticipants (const QList<QObject*>&) override;
		void mucSubjectChanged (const QString&) override;

		void gotMessage (QObject*) override;
		void statusChanged (const EntryStatus&, const QString&) override;
		void availableVariantsChanged (const QStringList&) override;
		void nameChanged (const QString&) override;
		void groupsChanged (const QStringList&) override;
		void chatPartStateChanged (const ChatPartState&, const QString&) override;
		void permsChanged () override;
		void entryGenerallyChanged () override;

		void nicknameConflict (const QString&) override;
		void beenKicked (const QString&) override;
		void beenBanned (const QString&) override;

		void gotBanListItem (const QString&,
				const QString&, const QDateTime&);
		void gotExceptListItem (const QString&,
				const QString&, const QDateTime&);
		void gotInviteListItem (const QString&,
				const QString&, const QDateTime&);
		void gotNewChannelModes (const ChannelModes&);
	};
}
