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
#include "channelhandler.h"
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
	: EntryBase (handler->GetIrcServerHandler ()->GetAccount ())
	,ICH_ (handler)
	{
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
			const QString&, const QString&)
	{
		return 0;
	}

	EntryStatus ChannelCLEntry::GetStatus (const QString&) const
	{
		return EntryStatus (SOnline, QString ());
	}

	bool ChannelCLEntry::MayChangeAffiliation (QObject* ,
			IMUCEntry::MUCAffiliation ) const
	{
		return false;
	}

	bool ChannelCLEntry::MayChangeRole(QObject* ,
			IMUCEntry::MUCRole ) const
	{
		return false;
	}

	IMUCEntry::MUCAffiliation
			ChannelCLEntry::GetAffiliation (QObject* ) const
	{

	}

	void ChannelCLEntry::SetAffiliation (QObject*,
			IMUCEntry::MUCAffiliation , const QString& )
	{
	}

	IMUCEntry::MUCRole ChannelCLEntry::GetRole (QObject* ) const
	{
	}

	void ChannelCLEntry::SetRole (QObject* ,
			IMUCEntry::MUCRole , const QString& )
	{
	}

	IMUCEntry::MUCFeatures ChannelCLEntry::GetMUCFeatures () const
	{
		return MUCFCanHaveSubject;
	}

	QString ChannelCLEntry::GetMUCSubject () const
	{
		return Subject_;
	}

	void ChannelCLEntry::SetMUCSubject (const QString& subject)
	{
		Subject_ = subject;
	}

	QList<QObject*> ChannelCLEntry::GetParticipants ()
	{
		return QList<QObject*> () << 0;
	}

	void ChannelCLEntry::Leave(const QString& )
	{
	}

	QString ChannelCLEntry::GetNick () const
	{
		return ICH_->GetIrcServerHandler ()->GetNickName ();
	}

	void ChannelCLEntry::SetNick (const QString& )
	{
	}

	QVariantMap ChannelCLEntry::GetIdentifyingData () const
	{
		return QVariantMap ();
	}

};
};
};
