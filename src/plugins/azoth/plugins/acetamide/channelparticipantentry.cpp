/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelparticipantentry.h"
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircmessage.h"
#include "ircaccount.h"
#include "channelsmanager.h"

namespace LC::Azoth::Acetamide
{
	ChannelParticipantEntry::ChannelParticipantEntry (const QString& nick,
			ChannelHandler *ich, IrcAccount *acc)
	: IrcParticipantEntry { nick, acc }
	, ICH_ { ich }
	{
		const auto infoMenu = new QMenu (tr ("Information"));
		infoMenu->addAction ("/WHOIS " + Nick_,
				this,
				[this] { ICH_->handleWhoIs (Nick_); });
		infoMenu->addAction ("/WHOWAS " + Nick_,
				this,
				[this] { ICH_->handleWhoWas (Nick_); });
		infoMenu->addAction ("/WHO " + Nick_,
				this,
				[this] { ICH_->handleWho (Nick_); });

		const auto ctcpMenu = new QMenu (QStringLiteral ("CTCP"));
		ctcpMenu->addAction (QStringLiteral ("PING"));
		ctcpMenu->addAction (QStringLiteral ("FINGER"));
		ctcpMenu->addAction (QStringLiteral ("VERSION"));
		ctcpMenu->addAction (QStringLiteral ("USERINFO"));
		ctcpMenu->addAction (QStringLiteral ("CLIENTINFO"));
		ctcpMenu->addAction (QStringLiteral ("SOURCE"));
		ctcpMenu->addAction (QStringLiteral ("TIME"));

		connect (ctcpMenu,
				&QMenu::triggered,
				[this] (QAction *act)
				{
					ICH_->handleCTCPRequest ({ Nick_, act->text ().toLower () });
				});

		Actions_ = QList { infoMenu->menuAction (), ctcpMenu->menuAction () };

		ServerID_ = ICH_->GetParentID ();
	}

	ICLEntry* ChannelParticipantEntry::GetParentCLEntry () const
	{
		return ICH_->GetCLEntry ();
	}

	QString ChannelParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ServerID_ + "_" + Nick_;
	}

	QString ChannelParticipantEntry::GetHumanReadableID () const
	{
		return Nick_ + "_" + ICH_->GetChannelID ();
	}

	QStringList ChannelParticipantEntry::Groups () const
	{
		return { ICH_->GetChannelID () };
	}

	void ChannelParticipantEntry::SetGroups (const QStringList&)
	{
	}

	IMessage* ChannelParticipantEntry::CreateMessage (IMessage::Type,
			const QString&, const QString& body)
	{
		const auto message = new IrcMessage (IMessage::Type::ChatMessage,
				IMessage::Direction::Out,
				ServerID_,
				Nick_,
				Account_->GetClientConnection ().get ());

		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());

		return message;
	}

	QList<ChannelRole> ChannelParticipantEntry::Roles () const
	{
		return Roles_;
	}

	ChannelRole ChannelParticipantEntry::HighestRole () const
	{
		if (Roles_.isEmpty ())
			return ChannelRole::Participant;

		return Roles_.last ();
	}

	void ChannelParticipantEntry::SetRole (ChannelRole role)
	{
		if (!Roles_.contains (role))
		{
			Roles_ << role;
			std::sort (Roles_.begin (), Roles_.end ());
			emit permsChanged ();
		}
	}

	void ChannelParticipantEntry::SetRoles (const QList<ChannelRole>& roles)
	{
		Roles_ = roles;
		std::sort (Roles_.begin (), Roles_.end ());
		emit permsChanged ();
	}

	void ChannelParticipantEntry::RemoveRole (ChannelRole role)
	{
		if (Roles_.removeAll (role))
			emit permsChanged ();
	}
}
