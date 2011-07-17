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
	
	QString ChannelCLEntry::GetRealID (QObject *participant) const
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
	//TODO Permissions
	QByteArray ChannelCLEntry::GetAffName (QObject*) const
	{
		return QByteArray ();
	}

	QMap<QByteArray, QByteArray> ChannelCLEntry::GetPerms (QObject*) const
	{
		return QMap<QByteArray, QByteArray> ();
	}

	void ChannelCLEntry::SetPerm (QObject*,
			const QByteArray&, const QByteArray&, const QString&)
	{
	}

	QMap<QByteArray, QList<QByteArray> > ChannelCLEntry::GetPossiblePerms () const
	{
		return QMap<QByteArray, QList<QByteArray> > ();
	}

	QString ChannelCLEntry::GetUserString (const QByteArray&) const
	{
		return QString ();
	}

	bool ChannelCLEntry::IsLessByPerm (QObject*, QObject*) const
	{
		return false;
	}

	bool ChannelCLEntry::MayChangePerm (QObject*,
			const QByteArray&, const QByteArray&) const
	{
		return false;
	}
};
};
};
