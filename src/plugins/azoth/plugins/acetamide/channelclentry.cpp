/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include "ircserverhandler.h"
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelCLEntry::ChannelCLEntry (ChannelHandler *handler)
	: QObject (handler->GetIrcServerHandler ()->GetAccount ())
	, ICH_ (handler)
	{
		Perms_ ["permclass_role"] << "owner";
		Perms_ ["permclass_role"] << "admin";
		Perms_ ["permclass_role"] << "pperator";
		Perms_ ["permclass_role"] << "half_operator";
		Perms_ ["permclass_role"] << "voiced";
		Perms_ ["permclass_role"] << "participant";
		Perms_ ["permclass_role"] << "norole";
		Perms_ ["permclass_role"] << "outcast";

		Role2Str_ [Owner] = "owner";
		Role2Str_ [Admin] = "admin";
		Role2Str_ [Operator] = "operator";
		Role2Str_ [HalfOperator] = "half_operator";
		Role2Str_ [Voiced] = "voiced";
		Role2Str_ [Participant] = "participant";
		Role2Str_ [NoRole] = "norole";
		Role2Str_ [Outcast] = "outcast";

		Translations_ ["permclass_role"] = tr ("Role");
		Translations_ ["owner"] = tr ("Owner");
		Translations_ ["admin"] = tr ("Admin");
		Translations_ ["operator"] = tr ("Operator");
		Translations_ ["half_operator"] = tr ("Half Operator");
		Translations_ ["voiced"] = tr ("Voiced");
		Translations_ ["participant"] = tr ("Participant");
		Translations_ ["norole"] = tr ("Kicked");
		Translations_ ["outcast"] = tr ("Banned");
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
		return ICH_->GetIrcServerHandler ()->GetAccount ();
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
		return ICH_->GetIrcServerHandler ()->GetAccount ()->
				GetAccountID () + "_" + ICH_->
				GetIrcServerHandler ()->GetServerID_ () + "_" +
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

	QMap<QString, QVariant>
			ChannelCLEntry::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
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
		ICH_->SetMUCSubject (subject);
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
		ICH_->LeaveChannel (msg, true);
	}

	QString ChannelCLEntry::GetNick () const
	{
		return ICH_->GetIrcServerHandler ()->GetNickName ();
	}

	void ChannelCLEntry::SetNick (const QString&)
	{
	}
	
	QString ChannelCLEntry::GetGroupName () const
	{
		return ICH_->GetSelf ()->Groups ().value (0);
	}

	QVariantMap ChannelCLEntry::GetIdentifyingData () const
	{
		QVariantMap result;
		result ["HumanReadableName"] = QString ("%1 on %2@%3:%4")
				.arg (ICH_->GetIrcServerHandler ()->GetNickName ())
				.arg (ICH_->GetChannelOptions ().ChannelName_)
				.arg (ICH_->GetChannelOptions ().ServerName_)
				.arg (ICH_->GetIrcServerHandler ()->
					GetServerOptions ().ServerPort_);
		result ["AccountID"] = ICH_->GetIrcServerHandler ()->
				GetAccount ()->GetAccountID ();
		result ["Nickname"] = ICH_->GetIrcServerHandler ()->
				GetNickName ();
		result ["Channel"] = ICH_->GetChannelOptions ().ChannelName_;
		result ["Server"] = ICH_->GetChannelOptions ().ServerName_;
		result ["Port"] = ICH_->GetIrcServerHandler ()->
				GetServerOptions ().ServerPort_;
		result ["Encoding"] = ICH_->GetIrcServerHandler ()->
				GetServerOptions ().ServerEncoding_;
		result ["SSL"] = ICH_->GetIrcServerHandler ()->
				GetServerOptions ().SSL_;

		return result;
	}

	void ChannelCLEntry::HandleMessage (ChannelPublicMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void ChannelCLEntry::HandleNewParticipants
			(const QList<ICLEntry*>& parts)
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

	QByteArray ChannelCLEntry::GetAffName (QObject*) const
	{
		return QByteArray ();
	}

	QMap<QByteArray, QByteArray>
			ChannelCLEntry::GetPerms (QObject *participant) const
	{
		if (!participant)
			participant = ICH_->GetSelf ().get ();

		QMap<QByteArray, QByteArray> result;
		ServerParticipantEntry *entry =
				qobject_cast<ServerParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ServerParticipantEntry";
			result ["permclass_role"] = "norole";
		}
		else
			result ["permclass_role"] =
					Role2Str_.value (entry->
							GetRole (ICH_->
								GetChannelOptions ().ChannelName_),
							"invalid");
		return result;
	}

	void ChannelCLEntry::SetPerm (QObject *participant,
			const QByteArray&,
			const QByteArray&,
			const QString&)
	{
		ServerParticipantEntry *entry =
				qobject_cast<ServerParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ServerParticipantEntry";
			return;
		}

// 		if (permClass == "permclass_role")
// 			ICH_->SetRole (entry, Role2Str_.key (perm), reason);
// 		else
// 		{
// 			qWarning () << Q_FUNC_INFO
// 			<< "unknown perm class"
// 			<< permClass;
// 			return;
// 		}
	}

	QMap<QByteArray, QList<QByteArray> >
			ChannelCLEntry::GetPossiblePerms () const
	{
		return Perms_;
	}

	QString ChannelCLEntry::GetUserString (const QByteArray& id) const
	{
		return Translations_.value (id, id);
	}

	bool ChannelCLEntry::IsLessByPerm
			(QObject *part1, QObject *part2) const
	{
		ServerParticipantEntry *e1 =
				qobject_cast<ServerParticipantEntry*> (part1);
		ServerParticipantEntry *e2 =
				qobject_cast<ServerParticipantEntry*> (part2);
		if (!e1 || !e2)
		{
			qWarning () << Q_FUNC_INFO
					<< part1
					<< "or"
					<< part2
					<< "is not a ServerParticipantEntry";
			return false;
		}

		return e1->GetRole (ICH_->GetChannelOptions ().ChannelName_) <
				e2->GetRole (ICH_->GetChannelOptions ().ChannelName_);
	}

	bool ChannelCLEntry::MayChange (ChannelRole ourRole,
			ServerParticipantEntry *entry,
			ChannelRole newRole) const
	{
		ChannelRole role = entry->
				GetRole (ICH_->GetChannelOptions ().ChannelName_);
		if (!ICH_->GetIrcServerHandler ()->IsRoleAvailable (ourRole) ||
				!ICH_->GetIrcServerHandler ()->IsRoleAvailable (newRole)
				|| !ICH_->GetIrcServerHandler ()->
					IsRoleAvailable (role))
			return false;

		if (ourRole == Participant ||
				ourRole == Voiced ||
				ourRole == NoRole ||
				ourRole == Outcast)
			return false;

		return true;
	}

	bool ChannelCLEntry::MayChangePerm (QObject *participant,
			const QByteArray& permClass, const QByteArray& perm) const
	{
		ServerParticipantEntry *entry =
				qobject_cast<ServerParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a ServerParticipantEntry";
			return false;
		}

		const ChannelRole ourRole = ICH_->GetSelf ()->
				GetRole (ICH_->GetChannelOptions ().ChannelName_);

		if (permClass == "permclass_role")
			return MayChange (ourRole, entry, Role2Str_.key (perm));
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown perm class"
					<< permClass;
			return false;
		}
	}
};
};
};
