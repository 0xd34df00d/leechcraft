/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelclentry.h"
#include <utility>
#include <util/sll/prelude.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/azothutil.h>
#include "channelhandler.h"
#include "channelpublicmessage.h"
#include "ircmessage.h"
#include "ircaccount.h"
#include "channelconfigwidget.h"
#include "channelsmanager.h"

namespace LC::Azoth::Acetamide
{
	ChannelCLEntry::ChannelCLEntry (ChannelHandler *handler)
	: ICH_ { handler }
	, Perms_
		{
			{ "permclass_managment", { "kick", "ban_by_name", "ban_by_user_and_domain", "ban_by_domain", "kick_and_ban" } },
			{ "permclass_role", { "participant" } },
		}
	, Managment2Str_
		{
			{ ChannelManagment::Kick, "kick" },
			{ ChannelManagment::BanByName, "ban_by_name" },
			{ ChannelManagment::BanByDomain, "ban_by_user_and_domain" },
			{ ChannelManagment::BanByUserAndDomain, "ban_by_domain" },
			{ ChannelManagment::KickAndBan, "kick_and_ban" },
		}
	, Translations_
		{
			{ "permclass_role", tr ("Role") },
			{ "participant", tr ("Participant") },
			{ "permclass_managment", tr ("Kick and Ban") },
			{ "kick", tr ("Kick") },
			{ "ban_by_name", tr ("Ban by nickname") },
			{ "ban_by_domain", tr ("Ban by mask (*!*@domain)") },
			{ "ban_by_user_and_domain", tr ("Ban by mask (*!user@domain)") },
			{ "kick_and_ban", tr ("Kick and ban") },
		}
	{
		Role2Str_ [ChannelRole::Participant] = "participant";
		Aff2Str_ [ChannelRole::Participant] = "noaffiliation";

		const auto& iSupport = ICH_->GetChannelsManager ()->GetISupport ();
		QString roles = iSupport [QStringLiteral ("PREFIX")].split (')').value (0);
		for (int i = roles.length () - 1; i >= 1; --i)
			switch (roles.at (i).toLatin1 ())
			{
			case 'v':
				Perms_ ["permclass_role"] << "voiced";
				Role2Str_ [ChannelRole::Voiced] = "voiced";
				Aff2Str_ [ChannelRole::Voiced] = "member";
				Translations_ ["voiced"] = tr ("Voiced");
				break;
			case 'h':
				Perms_ ["permclass_role"] << "halfoperator";
				Role2Str_ [ChannelRole::HalfOperator] = "halfoperator";
				Aff2Str_ [ChannelRole::HalfOperator] = "admin";
				Translations_ ["halfoperator"] = tr ("HalfOperator");
				break;
			case 'o':
				Perms_ ["permclass_role"] << "operator";
				Role2Str_ [ChannelRole::Operator] = "operator";
				Aff2Str_ [ChannelRole::Operator] = "admin";
				Translations_ ["operator"] = tr ("Operator");
				break;
			case 'a':
				Perms_ ["permclass_role"] << "admin";
				Role2Str_ [ChannelRole::Admin] = "administrator";
				Aff2Str_ [ChannelRole::Admin] = "admin";
				Translations_ ["admin"] = tr ("Admin");
				break;
			case 'q':
				Perms_ ["permclass_role"] << "owner";
				Role2Str_ [ChannelRole::Owner] = "owner";
				Aff2Str_ [ChannelRole::Owner] = "owner";
				Translations_ ["owner"] = tr ("Owner");
				break;
			}
	}

	ChannelHandler* ChannelCLEntry::GetChannelHandler () const
	{
		return ICH_;
	}

	QObject* ChannelCLEntry::GetQObject ()
	{
		return this;
	}

	IAccount* ChannelCLEntry::GetParentAccount () const
	{
		return ICH_->GetChannelsManager ()->GetAccount ();
	}

	ICLEntry::Features ChannelCLEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType ChannelCLEntry::GetEntryType () const
	{
		return EntryType::MUC;
	}

	QString ChannelCLEntry::GetEntryName () const
	{
		return ICH_->GetChannelID ();
	}

	void ChannelCLEntry::SetEntryName (const QString&)
	{
	}

	QString ChannelCLEntry::GetEntryID () const
	{
		return ICH_->GetChannelsManager ()->GetAccount ()->
				GetAccountID () + "_" +
				ICH_->GetChannelsManager ()->GetServerID () + "_" +
				ICH_->GetChannelID ();
	}

	QString ChannelCLEntry::GetHumanReadableID () const
	{
		return ICH_->GetChannelID ();
	}

	QStringList ChannelCLEntry::Groups () const
	{
		return { tr ("Channels") };
	}

	void ChannelCLEntry::SetGroups (const QStringList&)
	{
	}

	QStringList ChannelCLEntry::Variants () const
	{
		return { {} };
	}

	IMessage* ChannelCLEntry::CreateMessage (IMessage::Type,
			const QString& variant, const QString& body)
	{
		return variant.isEmpty () ?
				new ChannelPublicMessage (body, this) :
				nullptr;
	}

	QList<IMessage*> ChannelCLEntry::GetAllMessages () const
	{
		return AllMessages_;
	}

	void ChannelCLEntry::PurgeMessages (const QDateTime& before)
	{
		AzothUtil::StandardPurgeMessages (AllMessages_, before);
	}

	void ChannelCLEntry::SetChatPartState (ChatPartState, const QString&)
	{
	}

	QList<QAction*> ChannelCLEntry::GetActions () const
	{
		return {};
	}

	QMap<QString, QVariant> ChannelCLEntry::GetClientInfo (const QString&) const
	{
		return {};
	}

	void ChannelCLEntry::MarkMsgsRead ()
	{
		Core::Instance ().GetPluginProxy ()->MarkMessagesAsRead (this);
	}

	void ChannelCLEntry::ChatTabClosed ()
	{
	}

	QString ChannelCLEntry::GetRealID (QObject*) const
	{
		return {};
	}

	EntryStatus ChannelCLEntry::GetStatus (const QString&) const
	{
		return { SOnline, {} };
	}

	void ChannelCLEntry::ShowInfo ()
	{
	}

	IMUCEntry::MUCFeatures ChannelCLEntry::GetMUCFeatures () const
	{
		return MUCFCanHaveSubject;
	}

	QString ChannelCLEntry::GetMUCSubject () const
	{
		return ICH_->GetMUCSubject ();
	}

	bool ChannelCLEntry::CanChangeSubject () const
	{
		return !ICH_->GetChannelModes ().InviteMode_ ||
				ICH_->GetSelf ()->HighestRole () >= Operator;
	}

	void ChannelCLEntry::SetMUCSubject (const QString& subject)
	{
		ICH_->SetTopic (subject);
	}

	QList<QObject*> ChannelCLEntry::GetParticipants ()
	{
		return ICH_->GetParticipants ();
	}

	bool ChannelCLEntry::IsAutojoined () const
	{
		return false;
	}

	// TODO implement this
	void ChannelCLEntry::Join ()
	{
		qWarning () << Q_FUNC_INFO << "implement me!";
	}

	void ChannelCLEntry::Leave (const QString& msg)
	{
		ICH_->Leave (msg);
	}

	QString ChannelCLEntry::GetNick () const
	{
		return ICH_->GetChannelsManager ()->GetOurNick ();
	}

	void ChannelCLEntry::SetNick (const QString& nick)
	{
		ICH_->SendPublicMessage ("/nick " + nick);
	}

	QString ChannelCLEntry::GetGroupName () const
	{
		return ICH_->GetChannelID ();
	}

	QVariantMap ChannelCLEntry::GetIdentifyingData () const
	{
		const auto& channelOpts = ICH_->GetChannelOptions ();
		const auto& serverOpts = ICH_->GetChannelsManager ()->GetServerOptions ();
		const auto& name = QStringLiteral ("%1 on %2@%3:%4")
				.arg (GetNick (),
					  channelOpts.ChannelName_,
					  channelOpts.ServerName_)
				.arg (serverOpts.ServerPort_);
		return
		{
			{ Lits::HumanReadableName, name },
			{ Lits::AccountID, ICH_->GetChannelsManager ()->GetAccount ()->GetAccountID () },
			{ Lits::Nickname, GetNick () },
			{ Lits::Channel, channelOpts.ChannelName_ },
			{ Lits::Server, channelOpts.ServerName_ },
			{ Lits::Port, serverOpts.ServerPort_ },
			{ Lits::Encoding, serverOpts.ServerEncoding_ },
			{ Lits::SSL, serverOpts.SSL_ },
		};
	}

	void ChannelCLEntry::InviteToMUC (const QString&, const QString&)
	{
	}

	void ChannelCLEntry::HandleMessage (ChannelPublicMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	QByteArray ChannelCLEntry::GetAffName (QObject *participant) const
	{
		const auto entry = dynamic_cast<ChannelParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ChannelParticipantEntry";
			return "noaffiliation";
		}

		return Aff2Str_ [entry->HighestRole ()];
	}

	QMap<QByteArray, QList<QByteArray>>  ChannelCLEntry::GetPerms (QObject *participant) const
	{
		if (!participant)
			participant = ICH_->GetSelf ().get ();

		QMap<QByteArray, QList<QByteArray>>  result;
		const auto entry = dynamic_cast<ChannelParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ChannelParticipantEntry";
			result ["permclass_role"] << "norole";
		}
		else
			for (const auto& role : entry->Roles ())
				result ["permclass_role"] << Role2Str_.value (role, "invalid");

		return result;
	}

	QPair<QByteArray, QByteArray> ChannelCLEntry::GetKickPerm () const
	{
		return {};
	}

	QPair<QByteArray, QByteArray> ChannelCLEntry::GetBanPerm () const
	{
		return {};
	}

	void ChannelCLEntry::SetPerm (QObject *participant,
			const QByteArray& permClass,
			const QByteArray& perm,
			const QString& reason)
	{
		const auto entry = dynamic_cast<ChannelParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ChannelParticipantEntry";
			return;
		}

		if (permClass == "permclass_role")
			ICH_->SetRole (entry, Role2Str_.key (perm), reason);
		else if (permClass == "permclass_managment")
			ICH_->ManageWithParticipant (entry, Managment2Str_.key (perm));
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown perm class"
					<< permClass;
			return;
		}
	}

	QMap<QByteArray, QList<QByteArray>> ChannelCLEntry::GetPossiblePerms () const
	{
		return Perms_;
	}

	QString ChannelCLEntry::GetUserString (const QByteArray& id) const
	{
		return Translations_.value (id, id);
	}

	bool ChannelCLEntry::IsLessByPerm (QObject *p1, QObject *p2) const
	{
		const auto e1 = dynamic_cast<ChannelParticipantEntry*> (p1);
		const auto e2 = dynamic_cast<ChannelParticipantEntry*> (p2);
		if (!e1 || !e2)
		{
			qWarning () << Q_FUNC_INFO
					<< p1
					<< "or"
					<< p2
					<< "is not a ChannelParticipantEntry";
			return false;
		}

		return e1->HighestRole () < e2->HighestRole ();
	}

	namespace
	{
		bool MayChange (ChannelRole ourRole,
				ChannelParticipantEntry *entry,
				ChannelRole newRole)
		{
			ChannelRole role = entry->HighestRole ();

			if (ourRole < ChannelRole::HalfOperator)
				return false;

			if (ourRole == ChannelRole::Owner)
				return true;

			if (role > ourRole)
				return false;

			if (newRole > ourRole)
				return false;

			return true;
		}

		bool MayManage (ChannelRole ourRole,
				ChannelParticipantEntry *entry,
				const QString& nick)
		{
			ChannelRole role = entry->HighestRole ();

			if (ourRole < ChannelRole::HalfOperator)
				return false;

			if (ourRole == ChannelRole::Owner)
				return true;

			if (role > ourRole)
				return false;

			if (entry->GetEntryName () == nick)
				return false;

			return true;
		}
	}

	bool ChannelCLEntry::MayChangePerm (QObject *participant,
			const QByteArray& permClass, const QByteArray& perm) const
	{
		const auto entry = dynamic_cast<ChannelParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ChannelParticipantEntry";
			return false;
		}

		const auto ourRole = ICH_->GetSelf ()->HighestRole ();

		if (permClass == "permclass_role")
			return MayChange (ourRole, entry, Role2Str_.key (perm));
		if (permClass == "permclass_managment")
			return MayManage (ourRole, entry, ICH_->GetSelf ()->GetEntryName ());

		qWarning () << Q_FUNC_INFO
				<< "unknown perm class"
				<< permClass;
		return false;
	}

	bool ChannelCLEntry::IsMultiPerm (const QByteArray&) const
	{
		return true;
	}

	ChannelModes ChannelCLEntry::GetChannelModes () const
	{
		return ICH_->GetChannelModes ();
	}

	QWidget* ChannelCLEntry::GetConfigurationWidget ()
	{
		return new ChannelConfigWidget (this);
	}

	void ChannelCLEntry::AcceptConfiguration (QWidget *widget)
	{
		const auto cfg = qobject_cast<ChannelConfigWidget*> (widget);
		if (!cfg)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< widget
					<< "to ChannelConfigWidget";
			return;
		}

		cfg->accept ();
	}

	void ChannelCLEntry::RequestBanList ()
	{
		ICH_->RequestBanList ();
	}

	void ChannelCLEntry::RequestExceptList ()
	{
		ICH_->RequestExceptList ();
	}

	void ChannelCLEntry::RequestInviteList ()
	{
		ICH_->RequestInviteList ();
	}

	void ChannelCLEntry::SetBanListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		emit gotBanListItem (mask, nick, date);
	}

	void ChannelCLEntry::SetExceptListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		emit gotExceptListItem (mask, nick, date);
	}

	void ChannelCLEntry::SetInviteListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		emit gotInviteListItem (mask, nick, date);
	}

	void ChannelCLEntry::AddBanListItem (QString mask)
	{
		ICH_->AddBanListItem (std::move (mask));
	}

	void ChannelCLEntry::RemoveBanListItem (QString mask)
	{
		ICH_->RemoveBanListItem (std::move (mask));
	}

	void ChannelCLEntry::AddExceptListItem (QString mask)
	{
		ICH_->AddExceptListItem (std::move (mask));
	}

	void ChannelCLEntry::RemoveExceptListItem (QString mask)
	{
		ICH_->RemoveExceptListItem (std::move (mask));
	}

	void ChannelCLEntry::AddInviteListItem (QString mask)
	{
		ICH_->AddInviteListItem (std::move (mask));
	}

	void ChannelCLEntry::RemoveInviteListItem (QString mask)
	{
		ICH_->RemoveInviteListItem (std::move (mask));
	}

	void ChannelCLEntry::SetNewChannelModes (const ChannelModes& modes)
	{
		ICH_->SetNewChannelModes (modes);
	}

	QString ChannelCLEntry::Role2String (ChannelRole role) const
	{
		return Role2Str_ [role];
	}
}
