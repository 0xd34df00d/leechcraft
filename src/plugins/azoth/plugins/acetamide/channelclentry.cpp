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
#include <QImage>
#include <QtDebug>
#include "ircaccount.h"
#include "channelpublicmessage.h"
#include "channelhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelCLEntry::ChannelCLEntry (ChannelHandler *ch, IrcAccount *account)
	: QObject (ch)
	, Account_ (account)
	, CH_ (ch)
	{
	}

	ChannelHandler* ChannelCLEntry::GetChannelHandler () const
	{
		return CH_;
	}

	IrcAccount* ChannelCLEntry::GetIrcAccount () const
	{
		return Account_;
	}

	QObject* ChannelCLEntry::GetObject ()
	{
		return this;
	}

	QObject* ChannelCLEntry::GetParentAccount () const
	{
		return Account_;
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
		return CH_->GetChannelID ();
	}

	void ChannelCLEntry::SetEntryName (const QString&)
	{
	}

	QString ChannelCLEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + '_' + CH_->GetChannelID ();
	}

	QString ChannelCLEntry::GetHumanReadableID () const
	{
		return CH_->GetChannelID ();
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
			const QString& variant, const QString& text)
	{
		if (variant == "")
			return new ChannelPublicMessage (text, this);
		else
			return 0;
	}

	QList<QObject*> ChannelCLEntry::GetAllMessages () const
	{
		return AllMessages_;
	}

	QList<QAction*> ChannelCLEntry::GetActions () const
	{
		return QList<QAction*> ();
	}
	
	QMap<QString, QVariant> ChannelCLEntry::GetClientInfo (const QString&) const
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

	bool ChannelCLEntry::MayChangeAffiliation (QObject *entry, IMUCEntry::MUCAffiliation aff) const
	{
// 		MUCAffiliation ourAff = GetAffiliation (0);
// 		if (aff < MUCAAdmin)
// 			return false;
// 
// 		if (aff == MUCAOwner)
// 			return true;
// 
// 		MUCAffiliation partAff = GetAffiliation (entry);
// 		if (partAff >= aff)
// 			return false;
// 
// 		if (aff >= MUCAAdmin)
// 			return false;
// 
// 		return true;
		return false;
	}

	bool ChannelCLEntry::MayChangeRole (QObject *entry, IMUCEntry::MUCRole role) const
	{
// 		MUCAffiliation ourAff = GetAffiliation (0);
// 		MUCRole ourRole = GetRole (0);
// 
// 		MUCAffiliation aff = GetAffiliation (entry);
// 		MUCRole role = GetRole (entry);
// 
// 		if (role == MUCRInvalid ||
// 				ourRole == MUCRInvalid ||
// 				role == MUCRInvalid ||
// 				aff == MUCAInvalid ||
// 				ourAff == MUCAInvalid)
// 			return false;
// 
// 		if (ourRole != MUCRModerator)
// 			return false;
// 
// 		if (ourAff <= aff)
// 			return false;

// 		return true;
	//	return false;
	}

	IMUCEntry::MUCAffiliation ChannelCLEntry::GetAffiliation (QObject *entry) const
	{
// 		if (!entry)
// 			entry = CH_->GetSelf ();
// 
// 		ServerParticipantEntry *participant = qobject_cast<ServerParticipantEntry*> (entry);
// 		if (!participant)
// 		{
// 			qWarning () << Q_FUNC_INFO
// 					<< participant
// 					<< "is not a ServerParticipantEntry";
// 			return MUCAInvalid;
// 		}
// 
// 		return static_cast<MUCAffiliation> (participant->GetAffiliation (CH_->GetChannelOptions ().ChannelName_));
		return MUCAMember;
	}

	void ChannelCLEntry::SetAffiliation (QObject *participant, IMUCEntry::MUCAffiliation aff, const QString&)
	{
// 		ServerParticipantEntry *entry = qobject_cast<ServerParticipantEntry*> (participant);
// 		if (!entry)
// 		{
// 			qWarning () << Q_FUNC_INFO
// 					<< participant
// 					<< "is not a ServerParticipantEntry";
// 			return;
// 		}
// 
// 		CH_->SetAffiliation (entry, newAff, reason);
	}

	IMUCEntry::MUCRole ChannelCLEntry::GetRole (QObject*) const
	{
		return MUCRParticipant;
	}

	void ChannelCLEntry::SetRole (QObject*, IMUCEntry::MUCRole , const QString&)
	{
	}

	IMUCEntry::MUCFeatures ChannelCLEntry::GetMUCFeatures () const
	{
		return MUCFCanBeConfigured | MUCFCanHaveSubject;
	}

	QString ChannelCLEntry::GetMUCSubject () const
	{
		return CH_->GetSubject ();
	}

	void ChannelCLEntry::SetMUCSubject (const QString& subj)
	{
		CH_->SetSubject (subj);
	}

	QList<QObject*> ChannelCLEntry::GetParticipants ()
	{
		return CH_->GetParticipants ();
	}

	void ChannelCLEntry::Leave (const QString& msg)
	{
		CH_->Leave (msg);
	}

	QString ChannelCLEntry::GetNick () const
	{
		return CH_->GetNickname ();
	}

	void ChannelCLEntry::SetNick (const QString& nick)
	{
		CH_->SetNickname (nick);
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
};
};
};
