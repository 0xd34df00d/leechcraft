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

	void ChannelCLEntry::SetEntryName (const QString& name)
	{
	}

	QString ChannelCLEntry::GetEntryID () const
	{
		return CH_->GetChannelID ();
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
	
	QObject* ChannelCLEntry::CreateMessage (IMessage::MessageType type,
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
		return QMap <QString, QVariant> ();
	}

	EntryStatus ChannelCLEntry::GetStatus (const QString& variant) const
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

	bool ChannelCLEntry::MayChangeAffiliation (QObject*, IMUCEntry::MUCAffiliation) const
	{
		return false;
	}

	bool ChannelCLEntry::MayChangeRole (QObject*, IMUCEntry::MUCRole) const
	{
		return false;
	}

	IMUCEntry::MUCAffiliation ChannelCLEntry::GetAffiliation (QObject*) const
	{
		return MUCAMember;
	}

	void ChannelCLEntry::SetAffiliation (QObject*, IMUCEntry::MUCAffiliation , const QString&)
	{
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
