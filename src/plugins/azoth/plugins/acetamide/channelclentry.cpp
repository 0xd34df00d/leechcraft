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

#include "channelclentry.h"
#include <interfaces/azothutil.h>
#include "channelhandler.h"
#include "channelpublicmessage.h"
#include "ircmessage.h"
#include "ircaccount.h"
#include "channelconfigwidget.h"
#include "channelsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelCLEntry::ChannelCLEntry (ChannelHandler *handler)
	: ICH_ (handler)
	, IsWidgetRequest_ (false)
	{
		Perms_ ["permclass_managment"] << "kick";
		Perms_ ["permclass_managment"] << "ban_by_name";
		Perms_ ["permclass_managment"] << "ban_by_user_and_domain";
		Perms_ ["permclass_managment"] << "ban_by_domain";
		Perms_ ["permclass_managment"] << "kick_and_ban";

		Perms_ ["permclass_role"] << "participant";

		Role2Str_ [ChannelRole::Participant] = "participant";

		Aff2Str_ [ChannelRole::Participant] = "noaffiliation";

		Managment2Str_ [ChannelManagment::Kick] = "kick";
		Managment2Str_ [ChannelManagment::BanByName] = "ban_by_name";
		Managment2Str_ [ChannelManagment::BanByDomain] = "ban_by_user_and_domain";
		Managment2Str_ [ChannelManagment::BanByUserAndDomain] = "ban_by_domain";
		Managment2Str_ [ChannelManagment::KickAndBan] = "kick_and_ban";

		Translations_ ["permclass_role"] = tr ("Role");
		Translations_ ["participant"] = tr ("Participant");

		Translations_ ["permclass_managment"] = tr ("Kick and Ban");
		Translations_ ["kick"] = tr ("Kick");
		Translations_ ["ban_by_name"] = tr ("Ban by nickname");
		Translations_ ["ban_by_domain"] = tr ("Ban by mask (*!*@domain)");
		Translations_ ["ban_by_user_and_domain"] = tr ("Ban by mask (*!user@domain)");
		Translations_ ["kick_and_ban"] = tr ("Kick and ban");

		const auto& iSupport = ICH_->GetChannelsManager ()->GetISupport ();
		QString roles = iSupport ["PREFIX"].split (')').value (0);
		for (int i = roles.length () - 1; i >= 1; --i)
			switch (roles.at (i).toAscii ())
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

	QObject* ChannelCLEntry::GetObject ()
	{
		return this;
	}

	QObject* ChannelCLEntry::GetParentAccount () const
	{
		return ICH_->GetChannelsManager ()->GetAccount ();
	}

	ICLEntry::Features ChannelCLEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType ChannelCLEntry::GetEntryType () const
	{
		return ETMUC;
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
		return QStringList () << tr ("Channels");
	}

	void ChannelCLEntry::SetGroups (const QStringList&)
	{
	}

	QStringList ChannelCLEntry::Variants () const
	{
		QStringList result;
		result << "";
		return result;
	}

	QObject* ChannelCLEntry::CreateMessage (IMessage::MessageType,
			const QString& variant, const QString& body)
	{
		if (variant == "")
			return new ChannelPublicMessage (body, this);
		else
			return 0;
	}

	QList<QObject*> ChannelCLEntry::GetAllMessages () const
	{
		return AllMessages_;
	}

	void ChannelCLEntry::PurgeMessages (const QDateTime& before)
	{
		Util::StandardPurgeMessages (AllMessages_, before);
	}

	void ChannelCLEntry::SetChatPartState (ChatPartState, const QString&)
	{
	}

	QList<QAction*> ChannelCLEntry::GetActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> ChannelCLEntry::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}

	void ChannelCLEntry::MarkMsgsRead ()
	{
	}

	QString ChannelCLEntry::GetRealID (QObject*) const
	{
		return QString ();
	}

	EntryStatus ChannelCLEntry::GetStatus (const QString&) const
	{
		return EntryStatus (SOnline, QString ());
	}

	QImage ChannelCLEntry::GetAvatar () const
	{
		return QImage ();
	}

	QString ChannelCLEntry::GetRawInfo () const
	{
		return QString ();
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

	void ChannelCLEntry::SetMUCSubject (const QString& subject)
	{
		ICH_->SetTopic (subject);
	}

	QList<QObject*> ChannelCLEntry::GetParticipants ()
	{
		return ICH_->GetParticipants ();
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
		QVariantMap result;
		const auto& channelOpts = ICH_->GetChannelOptions ();
		const auto& serverOpts = ICH_->GetChannelsManager ()->GetServerOptions ();
		result ["HumanReadableName"] = QString ("%1 on %2@%3:%4")
				.arg (GetNick ())
				.arg (channelOpts.ChannelName_)
				.arg (channelOpts.ServerName_)
				.arg (serverOpts.ServerPort_);
		result ["AccountID"] = ICH_->GetChannelsManager ()->
				GetAccount ()->GetAccountID ();
		result ["Nickname"] = GetNick ();
		result ["Channel"] = channelOpts.ChannelName_;
		result ["Server"] = channelOpts.ServerName_;
		result ["Port"] = serverOpts.ServerPort_;
		result ["Encoding"] = serverOpts.ServerEncoding_;
		result ["SSL"] = serverOpts.SSL_;

		return result;
	}

	void ChannelCLEntry::InviteToMUC (const QString&, const QString&)
	{
	}

	void ChannelCLEntry::HandleMessage (ChannelPublicMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void ChannelCLEntry::HandleNewParticipants (const QList<ICLEntry*>& parts)
	{
		QObjectList objs;
		Q_FOREACH (ICLEntry *e, parts)
			objs << e->GetObject ();
		emit gotNewParticipants (objs);
	}

	void ChannelCLEntry::HandleSubjectChanged (const QString& subj)
	{
		emit mucSubjectChanged (subj);
	}

	QByteArray ChannelCLEntry::GetAffName (QObject *participant) const
	{
		ChannelParticipantEntry *entry = qobject_cast<ChannelParticipantEntry*> (participant);
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
		ChannelParticipantEntry *entry = qobject_cast<ChannelParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ChannelParticipantEntry";
			result ["permclass_role"] << "norole";
		}
		else
			Q_FOREACH (const ChannelRole& role, entry->Roles ())
				result ["permclass_role"] << Role2Str_.value (role, "invalid");

		return result;
	}

	void ChannelCLEntry::SetPerm (QObject *participant,
			const QByteArray& permClass,
			const QByteArray& perm,
			const QString& reason)
	{
		ChannelParticipantEntry *entry = qobject_cast<ChannelParticipantEntry*> (participant);
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
		ChannelParticipantEntry *e1 = qobject_cast<ChannelParticipantEntry*> (p1);
		ChannelParticipantEntry *e2 = qobject_cast<ChannelParticipantEntry*> (p2);
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
		ChannelParticipantEntry *entry = qobject_cast<ChannelParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ChannelParticipantEntry";
			return false;
		}

		const ChannelRole ourRole = ICH_->GetSelf ()->HighestRole ();

		if (permClass == "permclass_role")
			return MayChange (ourRole, entry, Role2Str_.key (perm));
		else if (permClass == "permclass_managment")
			return MayManage (ourRole, entry, ICH_->GetSelf ()->GetEntryName ());
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown perm class"
					<< permClass;
			return false;
		}
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
		ChannelConfigWidget *cfg = qobject_cast<ChannelConfigWidget*> (widget);
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

	void ChannelCLEntry::SetIsWidgetRequest (bool set)
	{
		IsWidgetRequest_ = set;
	}

	bool ChannelCLEntry::GetIsWidgetRequest () const
	{
		return IsWidgetRequest_;
	}

	void ChannelCLEntry::AddBanListItem (QString mask)
	{
		ICH_->AddBanListItem (mask);
	}

	void ChannelCLEntry::RemoveBanListItem (QString mask)
	{
		ICH_->RemoveBanListItem (mask);
	}

	void ChannelCLEntry::AddExceptListItem (QString mask)
	{
		ICH_->AddExceptListItem (mask);
	}

	void ChannelCLEntry::RemoveExceptListItem (QString mask)
	{
		ICH_->RemoveExceptListItem (mask);
	}

	void ChannelCLEntry::AddInviteListItem (QString mask)
	{
		ICH_->AddInviteListItem (mask);
	}

	void ChannelCLEntry::RemoveInviteListItem (QString mask)
	{
		ICH_->RemoveInviteListItem (mask);
	}

	void ChannelCLEntry::SetNewChannelModes (const ChannelModes& modes)
	{
		ICH_->SetNewChannelModes (modes);
	}

	QString ChannelCLEntry::Role2String (const ChannelRole& role) const
	{
		return Role2Str_ [role];
	}
}
}
}
