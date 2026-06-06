/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QStringList>
#include <QXmppMucIq.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "roomparticipantentry.h"
#include "glooxaccount.h"

class QXmppBookmarkSet;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class RoomPublicMessage;
	class RoomHandler;

	class RoomCLEntry : public QObject
					  , public ICLEntry
					  , public IMUCEntry
					  , public IMUCPerms
					  , public IConfigurableMUC
					  , public IHaveDirectedStatus
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry
						LC::Azoth::IMUCEntry
						LC::Azoth::IMUCPerms
						LC::Azoth::IConfigurableMUC
						LC::Azoth::IHaveDirectedStatus)

		friend class RoomHandler;

		const bool IsAutojoined_;
		GlooxAccount *Account_;
		QList<IMessage*> AllMessages_;
		RoomHandler *RH_;

		const MUCPerms_t Perms_;
		const QMap<QXmppMucItem::Role, QByteArray> Role2Str_;
		const QMap<QXmppMucItem::Affiliation, QByteArray> Aff2Str_;
		const QMap<QByteArray, QString> Translations_;

		mutable QAction *ActionRequestVoice_ = nullptr;
	public:
		RoomCLEntry (RoomHandler*, bool, GlooxAccount*);

		RoomHandler* GetRoomHandler () const;

		// ICLEntry
		QObject* GetQObject () override;
		GlooxAccount* GetParentAccount () const override;
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;
		QString GetEntryName () const override;
		void SetEntryName (const QString&) override;
		std::optional<EntryPersistentId> GetPersistentID () const override;
		EntryConventionalId GetConventionalID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList&) override;
		QStringList Variants () const override;
		void SendMessage (const OutgoingMessage&) override;
		QList<IMessage*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime&) override;
		void SetChatPartState (ChatPartState, const QString&) override;
		EntryStatus GetStatus (const QString&) const override;
		QList<QAction*> GetActions () const override;
		void ShowInfo () override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;
		void MarkMsgsRead () override;
		void ChatTabClosed () override;

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const override;
		QString GetMUCSubject () const override;
		bool CanChangeSubject () const override;
		void SetMUCSubject (const QString&) override;
		QList<ICLEntry*> GetParticipants () override;
		bool IsAutojoined () const override;
		void Join () override;
		void Leave (const QString&) override;
		QString GetNick () const override;
		void SetNick (const QString&) override;
		QString GetGroupName () const override;
		QVariantMap GetIdentifyingData () const override;
		QString GetRealID (const ICLEntry&) const override;
		void InviteToMUC (const QString&, const QString&) override;

		// IMUCPerms
		MUCPerms_t GetPossiblePerms () const override;
		MUCPerms_t GetPerms (const ICLEntry&) const override;
		QPair<QByteArray, QByteArray> GetKickPerm () const override;
		QPair<QByteArray, QByteArray> GetBanPerm () const override;
		QByteArray GetAffName (const ICLEntry&) const override;
		bool MayChangePerm (const ICLEntry&, const QByteArray&, const QByteArray&) const override;
		void SetPerm (ICLEntry&, const QByteArray&, const QByteArray&, const QString&) override;
		void TrySetPerm (const QString&, const QByteArray&, const QByteArray&, const QString&) override;
		bool IsLessByPerm (const ICLEntry&, const ICLEntry&) const override;
		bool IsMultiPerm (const QByteArray&) const override;
		QString GetUserString (const QByteArray&) const override;

		// IConfigurableMUC
		QWidget* GetConfigurationWidget () override;
		void AcceptConfiguration (QWidget*) override;

		// IHaveDirectedStatus
		bool CanSendDirectedStatusNow (const QString&) override;
		void SendDirectedStatus (const EntryStatus&, const QString&) override;

		void MoveMessages (const RoomParticipantEntry_ptr& from, const RoomParticipantEntry_ptr& to);

		void HandleMessage (RoomPublicMessage*);
	private slots:
		void handleBookmarks (const QXmppBookmarkSet&);
	};
}
}
}
