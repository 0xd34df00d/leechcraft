/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverresponsemanager.h"
#include <util/sll/prelude.h>
#include <util/sll/functional.h>
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include "ircserverhandler.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "ircaccount.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ServerResponseManager::ServerResponseManager (IrcServerHandler *ish)
	: QObject (ish)
	, ISH_ (ish)
	{
		using Util::BindMemFn;

		Command2Action_ ["join"] = BindMemFn (&ServerResponseManager::GotJoin, this);
		Command2Action_ ["part"] = BindMemFn (&ServerResponseManager::GotPart, this);
		Command2Action_ ["quit"] = BindMemFn (&ServerResponseManager::GotQuit, this);
		Command2Action_ ["privmsg"] = BindMemFn (&ServerResponseManager::GotPrivMsg, this);
		Command2Action_ ["notice"] = BindMemFn (&ServerResponseManager::GotNoticeMsg, this);
		Command2Action_ ["nick"] = BindMemFn (&ServerResponseManager::GotNick, this);
		Command2Action_ ["ping"] = BindMemFn (&ServerResponseManager::GotPing, this);
		Command2Action_ ["topic"] = BindMemFn (&ServerResponseManager::GotTopic, this);
		Command2Action_ ["kick"] = BindMemFn (&ServerResponseManager::GotKick, this);
		Command2Action_ ["invite"] = BindMemFn (&ServerResponseManager::GotInvitation, this);
		Command2Action_ ["ctcp_rpl"] = BindMemFn (&ServerResponseManager::GotCTCPReply, this);
		Command2Action_ ["ctcp_rqst"] = BindMemFn (&ServerResponseManager::GotCTCPRequestResult, this);
		Command2Action_ ["331"] = BindMemFn (&ServerResponseManager::GotTopic, this);
		Command2Action_ ["332"] = BindMemFn (&ServerResponseManager::GotTopic, this);
		Command2Action_ ["341"] = BindMemFn (&ServerResponseManager::ShowInviteMessage, this);
		Command2Action_ ["353"] = BindMemFn (&ServerResponseManager::GotNames, this);
		Command2Action_ ["366"] = BindMemFn (&ServerResponseManager::GotEndOfNames, this);
		Command2Action_ ["301"] = BindMemFn (&ServerResponseManager::GotAwayReply, this);
		Command2Action_ ["305"] = BindMemFn (&ServerResponseManager::GotSetAway, this);
		Command2Action_ ["306"] = BindMemFn (&ServerResponseManager::GotSetAway, this);
		Command2Action_ ["302"] = BindMemFn (&ServerResponseManager::GotUserHost, this);
		Command2Action_ ["303"] = BindMemFn (&ServerResponseManager::GotIson, this);
		Command2Action_ ["311"] = BindMemFn (&ServerResponseManager::GotWhoIsUser, this);
		Command2Action_ ["312"] = BindMemFn (&ServerResponseManager::GotWhoIsServer, this);
		Command2Action_ ["313"] = BindMemFn (&ServerResponseManager::GotWhoIsOperator, this);
		Command2Action_ ["317"] = BindMemFn (&ServerResponseManager::GotWhoIsIdle, this);
		Command2Action_ ["318"] = BindMemFn (&ServerResponseManager::GotEndOfWhoIs, this);
		Command2Action_ ["319"] = BindMemFn (&ServerResponseManager::GotWhoIsChannels, this);
		Command2Action_ ["314"] = BindMemFn (&ServerResponseManager::GotWhoWas, this);
		Command2Action_ ["369"] = BindMemFn (&ServerResponseManager::GotEndOfWhoWas, this);
		Command2Action_ ["352"] = BindMemFn (&ServerResponseManager::GotWho, this);
		Command2Action_ ["315"] = BindMemFn (&ServerResponseManager::GotEndOfWho, this);
		Command2Action_ ["342"] = BindMemFn (&ServerResponseManager::GotSummoning, this);
		Command2Action_ ["351"] = BindMemFn (&ServerResponseManager::GotVersion, this);
		Command2Action_ ["364"] = BindMemFn (&ServerResponseManager::GotLinks, this);
		Command2Action_ ["365"] = BindMemFn (&ServerResponseManager::GotEndOfLinks, this);
		Command2Action_ ["371"] = BindMemFn (&ServerResponseManager::GotInfo, this);
		Command2Action_ ["374"] = BindMemFn (&ServerResponseManager::GotEndOfInfo, this);
		Command2Action_ ["372"] = BindMemFn (&ServerResponseManager::GotMotd, this);
		Command2Action_ ["375"] = BindMemFn (&ServerResponseManager::GotMotd, this);
		Command2Action_ ["376"] = BindMemFn (&ServerResponseManager::GotEndOfMotd, this);
		Command2Action_ ["422"] = BindMemFn (&ServerResponseManager::GotMotd, this);
		Command2Action_ ["381"] = BindMemFn (&ServerResponseManager::GotYoureOper, this);
		Command2Action_ ["382"] = BindMemFn (&ServerResponseManager::GotRehash, this);
		Command2Action_ ["391"] = BindMemFn (&ServerResponseManager::GotTime, this);
		Command2Action_ ["251"] = BindMemFn (&ServerResponseManager::GotLuserOnlyMsg, this);
		Command2Action_ ["252"] = BindMemFn (&ServerResponseManager::GotLuserParamsWithMsg, this);
		Command2Action_ ["253"] = BindMemFn (&ServerResponseManager::GotLuserParamsWithMsg, this);
		Command2Action_ ["254"] = BindMemFn (&ServerResponseManager::GotLuserParamsWithMsg, this);
		Command2Action_ ["255"] = BindMemFn (&ServerResponseManager::GotLuserOnlyMsg, this);
		Command2Action_ ["392"] = BindMemFn (&ServerResponseManager::GotUsersStart, this);
		Command2Action_ ["393"] = BindMemFn (&ServerResponseManager::GotUsers, this);
		Command2Action_ ["395"] = BindMemFn (&ServerResponseManager::GotNoUser, this);
		Command2Action_ ["394"] = BindMemFn (&ServerResponseManager::GotEndOfUsers, this);
		Command2Action_ ["200"] = BindMemFn (&ServerResponseManager::GotTraceLink, this);
		Command2Action_ ["201"] = BindMemFn (&ServerResponseManager::GotTraceConnecting, this);
		Command2Action_ ["202"] = BindMemFn (&ServerResponseManager::GotTraceHandshake, this);
		Command2Action_ ["203"] = BindMemFn (&ServerResponseManager::GotTraceUnknown, this);
		Command2Action_ ["204"] = BindMemFn (&ServerResponseManager::GotTraceOperator, this);
		Command2Action_ ["205"] = BindMemFn (&ServerResponseManager::GotTraceUser, this);
		Command2Action_ ["206"] = BindMemFn (&ServerResponseManager::GotTraceServer, this);
		Command2Action_ ["207"] = BindMemFn (&ServerResponseManager::GotTraceService, this);
		Command2Action_ ["208"] = BindMemFn (&ServerResponseManager::GotTraceNewType, this);
		Command2Action_ ["209"] = BindMemFn (&ServerResponseManager::GotTraceClass, this);
		Command2Action_ ["261"] = BindMemFn (&ServerResponseManager::GotTraceLog, this);
		Command2Action_ ["262"] = BindMemFn (&ServerResponseManager::GotTraceEnd, this);
		Command2Action_ ["211"] = BindMemFn (&ServerResponseManager::GotStatsLinkInfo, this);
		Command2Action_ ["212"] = BindMemFn (&ServerResponseManager::GotStatsCommands, this);
		Command2Action_ ["219"] = BindMemFn (&ServerResponseManager::GotStatsEnd, this);
		Command2Action_ ["242"] = BindMemFn (&ServerResponseManager::GotStatsUptime, this);
		Command2Action_ ["243"] = BindMemFn (&ServerResponseManager::GotStatsOline, this);
		Command2Action_ ["256"] = BindMemFn (&ServerResponseManager::GotAdmineMe, this);
		Command2Action_ ["257"] = BindMemFn (&ServerResponseManager::GotAdminLoc1, this);
		Command2Action_ ["258"] = BindMemFn (&ServerResponseManager::GotAdminLoc2, this);
		Command2Action_ ["259"] = BindMemFn (&ServerResponseManager::GotAdminEmail, this);
		Command2Action_ ["263"] = BindMemFn (&ServerResponseManager::GotTryAgain, this);
		Command2Action_ ["005"] = BindMemFn (&ServerResponseManager::GotISupport, this);
		Command2Action_ ["mode"] = BindMemFn (&ServerResponseManager::GotChannelMode, this);
		Command2Action_ ["367"] = BindMemFn (&ServerResponseManager::GotBanList, this);
		Command2Action_ ["368"] = BindMemFn (&ServerResponseManager::GotBanListEnd, this);
		Command2Action_ ["348"] = BindMemFn (&ServerResponseManager::GotExceptList, this);
		Command2Action_ ["349"] = BindMemFn (&ServerResponseManager::GotExceptListEnd, this);
		Command2Action_ ["346"] = BindMemFn (&ServerResponseManager::GotInviteList, this);
		Command2Action_ ["347"] = BindMemFn (&ServerResponseManager::GotInviteListEnd, this);
		Command2Action_ ["324"] = BindMemFn (&ServerResponseManager::GotChannelModes, this);
		Command2Action_ ["321"] = BindMemFn (&IrcServerHandler::GotChannelsListBegin, ISH_);
		Command2Action_ ["322"] = BindMemFn (&IrcServerHandler::GotChannelsList, ISH_);
		Command2Action_ ["323"] = BindMemFn (&IrcServerHandler::GotChannelsListEnd, ISH_);

		//not from rfc
		Command2Action_ ["330"] = BindMemFn (&ServerResponseManager::GotWhoIsAccount, this);
		Command2Action_ ["671"] = BindMemFn (&ServerResponseManager::GotWhoIsSecure, this);
		Command2Action_ ["328"] = BindMemFn (&ServerResponseManager::GotChannelUrl, this);
		Command2Action_ ["333"] = BindMemFn (&ServerResponseManager::GotTopicWhoTime, this);
		Command2Action_ ["004"] = BindMemFn (&ServerResponseManager::GotServerInfo, this);
		Command2Action_ ["307"] = [this] (const IrcMessageOptions& opts) { ISH_->ShowAnswer ("307", opts.Message_); };
		Command2Action_ ["310"] = [this] (const IrcMessageOptions& opts) { ISH_->ShowAnswer ("310", opts.Message_); };
		Command2Action_ ["320"] = [this] (const IrcMessageOptions& opts) { ISH_->ShowAnswer ("320", opts.Message_); };
		Command2Action_ ["378"] = [this] (const IrcMessageOptions& opts) { ISH_->ShowAnswer ("278", opts.Message_); };

		MatchString2Server_ ["unreal"] = IrcServer::UnrealIRCD;
	}

	void ServerResponseManager::DoAction (const IrcMessageOptions& opts)
	{
		if (opts.Command_ == "privmsg" && IsCTCPMessage (opts.Message_))
			Command2Action_ ["ctcp_rpl"] (opts);
		else if (opts.Command_ == "notice" && IsCTCPMessage (opts.Message_))
			Command2Action_ ["ctcp_rqst"] (opts);
		else if (Command2Action_.contains (opts.Command_))
			Command2Action_ [opts.Command_] (opts);
		else
			ISH_->ShowAnswer ("UNKNOWN CMD " + opts.Command_, opts.Message_);
	}

	bool ServerResponseManager::IsCTCPMessage (const QString& msg)
	{
		return msg.startsWith ('\001') && msg.endsWith ('\001');
	}

	void ServerResponseManager::GotJoin (const IrcMessageOptions& opts)
	{
		const QString& channel = opts.Message_.isEmpty () ?
				QString::fromStdString (opts.Parameters_.last ()) :
				opts.Message_;

		if (opts.Nick_ == ISH_->GetNickName ())
		{
			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ServerName_ = ISH_->GetServerOptions ().ServerName_.toLower ();
			co.ChannelPassword_ = QString ();
			ISH_->JoinedChannel (co);
		}
		else
			ISH_->JoinParticipant (opts.Nick_, channel, opts.Host_, opts.UserName_);
	}

	void ServerResponseManager::GotPart (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

        const QString channel (QString::fromStdString (opts.Parameters_.first ()));
		if (opts.Nick_ == ISH_->GetNickName ())
		{
			ISH_->CloseChannel (channel);
			return;
		}

		ISH_->LeaveParticipant (opts.Nick_, channel, opts.Message_);
	}

	void ServerResponseManager::GotQuit (const IrcMessageOptions& opts)
	{
		if (opts.Nick_ == ISH_->GetNickName ())
			ISH_->DisconnectFromServer ();
		else
			ISH_->QuitParticipant (opts.Nick_, opts.Message_);
	}

	void ServerResponseManager::GotPrivMsg (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString target (QString::fromStdString (opts.Parameters_.first ()));
		ISH_->IncomingMessage (opts.Nick_, target, opts.Message_);
	}

	void ServerResponseManager::GotNoticeMsg (const IrcMessageOptions& opts)
	{
		ISH_->IncomingNoticeMessage (opts.Nick_, opts.Message_);
	}

	void ServerResponseManager::GotNick (const IrcMessageOptions& opts)
	{
		ISH_->ChangeNickname (opts.Nick_, opts.Message_);
	}

	void ServerResponseManager::GotPing (const IrcMessageOptions& opts)
	{
		ISH_->PongMessage (opts.Message_);
	}

	void ServerResponseManager::GotTopic (const IrcMessageOptions& opts)
	{
        QString channel (QString::fromStdString (opts.Parameters_.last ()));
		ISH_->GotTopic (channel, opts.Message_);
	}

	void ServerResponseManager::GotKick (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel (QString::fromStdString (opts.Parameters_.first ()));
		const QString target (QString::fromStdString (opts.Parameters_.last ()));
		if (opts.Nick_ == target)
			return;

		ISH_->GotKickCommand (opts.Nick_, channel, target, opts.Message_);
	}

	void ServerResponseManager::GotInvitation (const IrcMessageOptions& opts)
	{
		if (XmlSettingsManager::Instance ().property ("ShowInviteDialog").toBool ())
			XmlSettingsManager::Instance ().setProperty ("InviteActionByDefault", 0);

		if (!XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt ())
			ISH_->GotInvitation (opts.Nick_, opts.Message_);
		else if (XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt () == 1)
			GotJoin (opts);

		ISH_->ShowAnswer ("invite", opts.Nick_ + tr (" invites you to a channel ") + opts.Message_);
	}

	void ServerResponseManager::ShowInviteMessage (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		QString msg = tr ("You invite ") + QString::fromStdString (opts.Parameters_.at (1)) +
				tr (" to a channel ") + QString::fromStdString (opts.Parameters_.at (2));
		ISH_->ShowAnswer ("invite", msg);
	}

	void ServerResponseManager::GotCTCPReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		if (opts.Message_.isEmpty ())
			return;

		QStringList ctcpList = opts.Message_.mid (1, opts.Message_.length () - 2).split (' ');
		if (ctcpList.isEmpty ())
			return;

		QString cmd;
		QString outputMessage;
		const QString& lcVer = GetProxyHolder ()->GetVersion ();
		const QString version = QString ("LeechCraft %1 (Acetamide 2.0)").arg (lcVer);
		const QDateTime currentDT = QDateTime::currentDateTime ();
		const QString firstPartOutput = QString ("LeechCraft %1 (Acetamide 2.0) - "
					"https://leechcraft.org")
				.arg (lcVer);
		const QString target = QString::fromStdString (opts.Parameters_.last ());

		if (ctcpList.at (0).toLower () == "version")
		{
			cmd = QString ("%1 %2%3").arg ("\001VERSION",
					version, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("VERSION", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "ping")
		{
			cmd = QString ("%1 %2%3").arg ("\001PING ",
					QString::number (currentDT.toSecsSinceEpoch ()), QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("PING", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "time")
		{
			cmd = QString ("%1 %2%3").arg ("\001TIME",
					currentDT.toString ("ddd MMM dd hh:mm:ss yyyy"),
					QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("TIME", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "source")
		{
			cmd = QString ("%1 %2 %3").arg ("\001SOURCE", firstPartOutput,
					QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("SOURCE", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "clientinfo")
		{
			cmd = QString ("%1 %2 - %3 %4 %5").arg ("\001CLIENTINFO",
					firstPartOutput, "Supported tags:",
					"VERSION PING TIME SOURCE CLIENTINFO", QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("CLIENTINFO", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "action")
		{
			QString mess = "/me " + QStringList (ctcpList.mid (1)).join (" ");
			ISH_->IncomingMessage (opts.Nick_, target, mess);
			return;
		}

		if (outputMessage.isEmpty ())
			return;

		ISH_->CTCPReply (opts.Nick_, cmd, outputMessage);
	}

	void ServerResponseManager::GotCTCPRequestResult (const IrcMessageOptions& opts)
	{
		if (QString::fromStdString (opts.Parameters_.first ()) != ISH_->GetNickName ())
			return;

		if (opts.Message_.isEmpty ())
			return;

		const QStringList ctcpList = opts.Message_.mid (1, opts.Message_.length () - 2).split (' ');
		if (ctcpList.isEmpty ())
			return;

		const QString output = tr ("Received answer CTCP-%1 from %2: %3")
				.arg (ctcpList.at (0), opts.Nick_,
						(QStringList (ctcpList.mid (1))).join (" "));
		ISH_->CTCPRequestResult (output);
	}

	void ServerResponseManager::GotNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel = QString::fromStdString (opts.Parameters_.last ());
		const QStringList& participants = opts.Message_.split (' ');
		ISH_->GotNames (channel, participants);
	}

	void ServerResponseManager::GotEndOfNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel = QString::fromStdString (opts.Parameters_.last ());
		ISH_->GotEndOfNames (channel);
	}

	void ServerResponseManager::GotAwayReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString& target = QString::fromStdString (opts.Parameters_.last ());
		ISH_->IncomingMessage (target, target, QString ("[AWAY] %1 :%2")
				.arg (target, opts.Message_), IMessage::Type::StatusMessage);
	}

	void ServerResponseManager::GotSetAway (const IrcMessageOptions& opts)
	{
		switch (opts.Command_.toInt ())
		{
		case 305:
			ISH_->ChangeAway (false);
			break;
		case 306:
			ISH_->ChangeAway (true, opts.Message_);
			break;
		}

		ISH_->ShowAnswer ("away", opts.Message_, true, IMessage::Type::StatusMessage);
	}

	void ServerResponseManager::GotUserHost (const IrcMessageOptions& opts)
	{
		for (const auto& str : opts.Message_.splitRef (' '))
		{
			const auto& user = str.left (str.indexOf ('='));
			const auto& host = str.mid (str.indexOf ('=') + 1);
			ISH_->ShowUserHost (user.toString (), host.toString ());
		}
	}

	void ServerResponseManager::GotIson (const IrcMessageOptions& opts)
	{
		for (const auto& str : opts.Message_.splitRef (' '))
			ISH_->ShowIsUserOnServer (str.toString ());
	}

	void ServerResponseManager::GotWhoIsUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.UserName_ = QString::fromStdString (opts.Parameters_.at (2));
		msg.Host_ = QString::fromStdString (opts.Parameters_.at (3));
		msg.RealName_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.ServerName_ = QString::fromStdString (opts.Parameters_.at (2));
		msg.ServerCountry_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.IrcOperator_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsIdle (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.IdleTime_ = Util::MakeTimeFromLong (std::stol (opts.Parameters_.at (2)));
		msg.AuthTime_ = QDateTime::fromSecsSinceEpoch (std::stoul (opts.Parameters_.at (3))).toString (Qt::TextDate);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotEndOfWhoIs (const IrcMessageOptions& opts)
	{
		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg, true);
	}

	void ServerResponseManager::GotWhoIsChannels (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.Channels_ = opts.Message_.split (' ', Qt::SkipEmptyParts);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoWas (const IrcMessageOptions& opts)
	{
		const QString message = QString::fromStdString (opts.Parameters_.at (1)) +
				" - " + QString::fromStdString (opts.Parameters_.at (2)) + "@"
				+ QString::fromStdString (opts.Parameters_.at (3)) +
				" (" + opts.Message_ + ")";
		ISH_->ShowWhoWasReply (message);
	}

	void ServerResponseManager::GotEndOfWhoWas (const IrcMessageOptions& opts)
	{
        ISH_->ShowWhoWasReply (opts.Message_, true);
	}

	void ServerResponseManager::GotWho (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		WhoMessage msg;
		msg.Channel_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.UserName_ = QString::fromStdString (opts.Parameters_.at (2));
		msg.Host_ = QString::fromStdString (opts.Parameters_.at (3));
		msg.ServerName_ = QString::fromStdString (opts.Parameters_.at (4));
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (5));
		int index = opts.Message_.indexOf (' ');
		msg.RealName_ = opts.Message_.mid (index);
		msg.Jumps_ = opts.Message_.left (index).toInt ();
		msg.Flags_ = QString::fromStdString (opts.Parameters_.at (6));
		if (msg.Flags_.at (0) == 'H')
			msg.IsAway_ = false;
		else if (msg.Flags_.at (0) == 'G')
			msg.IsAway_ = true;
		ISH_->ShowWhoReply (msg);
	}

	void ServerResponseManager::GotEndOfWho (const IrcMessageOptions& opts)
	{
		WhoMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoReply (msg, true);
	}

	void ServerResponseManager::GotSummoning (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->ShowAnswer ("summon", QString::fromStdString (opts.Parameters_.at (1)) +
				tr (" summoning to IRC"));
	}

	namespace
	{
		QString BuildParamsStr (const QList<std::string>& params)
		{
			QString string;
			for (const auto& str : params)
				string.append (QString::fromStdString (str) + " ");
			return string;
		}

		template<int N>
		QString BuildParamsStr (const IrcMessageOptions& opts)
		{
			return BuildParamsStr (opts.Parameters_.mid (N));
		}

		template<>
		QString BuildParamsStr<0> (const IrcMessageOptions& opts)
		{
			return BuildParamsStr (opts.Parameters_);
		}
	}

	void ServerResponseManager::GotVersion (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("version", BuildParamsStr<0> (opts) + opts.Message_);
	}

	void ServerResponseManager::GotLinks (const IrcMessageOptions& opts)
	{
		ISH_->ShowLinksReply (BuildParamsStr<1> (opts) + opts.Message_);
	}

	void ServerResponseManager::GotEndOfLinks (const IrcMessageOptions& opts)
	{
        ISH_->ShowLinksReply (opts.Message_, true);
	}

	void ServerResponseManager::GotInfo (const IrcMessageOptions& opts)
	{
		ISH_->ShowInfoReply (opts.Message_);
	}

	void ServerResponseManager::GotEndOfInfo (const IrcMessageOptions& opts)
	{
        ISH_->ShowInfoReply (opts.Message_, true);
	}

	void ServerResponseManager::GotMotd (const IrcMessageOptions& opts)
	{
		ISH_->ShowMotdReply (opts.Message_);
	}

	void ServerResponseManager::GotEndOfMotd (const IrcMessageOptions& opts)
	{
        ISH_->ShowMotdReply (opts.Message_, true);
	}

	void ServerResponseManager::GotYoureOper (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("oper", opts.Message_);
	}

	void ServerResponseManager::GotRehash (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("rehash", QString::fromStdString (opts.Parameters_.last ()) +
			" :" + opts.Message_);
	}

	void ServerResponseManager::GotTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("time", QString::fromStdString (opts.Parameters_.last ()) +
				" :" + opts.Message_);
	}

	void ServerResponseManager::GotLuserOnlyMsg (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("luser", opts.Message_);
	}

	void ServerResponseManager::GotLuserParamsWithMsg (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("luser", QString::fromStdString (opts.Parameters_.last ()) + ":"
				+ opts.Message_);
	}

	void ServerResponseManager::GotUsersStart (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponseManager::GotUsers (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponseManager::GotNoUser (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponseManager::GotEndOfUsers (const IrcMessageOptions&)
	{
		ISH_->ShowUsersReply (tr ("End of USERS"), true);
	}

	void ServerResponseManager::GotTraceLink (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceConnecting (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("trace", BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceHandshake (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceUnknown (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceService (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceNewType (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceClass (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceLog (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& server = QString::fromStdString (opts.Parameters_.last ());
		ISH_->ShowTraceReply (server + " " + opts.Message_, true);
	}

	void ServerResponseManager::GotStatsLinkInfo (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowStatsReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotStatsCommands (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowStatsReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotStatsEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& letter = QString::fromStdString (opts.Parameters_.last ());
		ISH_->ShowStatsReply (letter + " " + opts.Message_, true);
	}

	void ServerResponseManager::GotStatsUptime (const IrcMessageOptions& opts)
	{
		ISH_->ShowStatsReply (opts.Message_);
	}

	void ServerResponseManager::GotStatsOline (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowStatsReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotAdmineMe (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("admin", QString::fromStdString (opts.Parameters_.last ()) + ":" + opts.Message_);
	}

	void ServerResponseManager::GotAdminLoc1 (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponseManager::GotAdminLoc2 (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponseManager::GotAdminEmail (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponseManager::GotTryAgain (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString cmd = QString::fromStdString (opts.Parameters_.last ());
		ISH_->ShowAnswer ("error", cmd + ":" + opts.Message_);
	}

	void ServerResponseManager::GotISupport (const IrcMessageOptions& opts)
	{
		ISH_->JoinFromQueue ();

		auto result = BuildParamsStr<0> (opts);
		result.append (":").append (opts.Message_);
		ISH_->ParserISupport (result);
		ISH_->ShowAnswer ("mode", result);
	}

	void ServerResponseManager::GotChannelMode (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		if (opts.Parameters_.count () == 1 &&
				QString::fromStdString (opts.Parameters_.first ()) == ISH_->GetNickName ())
		{
			ISH_->ParseUserMode (QString::fromStdString (opts.Parameters_.first ()),
					opts.Message_);
			return;
		}

		const QString channel = QString::fromStdString (opts.Parameters_.first ());

		if (opts.Parameters_.count () == 2)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (1)));
		else if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (1)),
					QString::fromStdString (opts.Parameters_.at (2)));
	}

	void ServerResponseManager::GotChannelModes (const IrcMessageOptions& opts)
	{
		const QString channel = QString::fromStdString (opts.Parameters_.at (1));

		if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (2)));
		else if (opts.Parameters_.count () == 4)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (2)),
					QString::fromStdString (opts.Parameters_.at (3)));
	}

	void ServerResponseManager::GotBanList (const IrcMessageOptions& opts)
	{
		const int count = opts.Parameters_.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;

		if (count > 2)
		{
			channel = QString::fromStdString (opts.Parameters_.at (1));
			mask = QString::fromStdString (opts.Parameters_.at (2));
		}

		if (count > 3)
		{
			QString name = QString::fromStdString (opts.Parameters_.at (3));
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromSecsSinceEpoch (std::stoi (opts.Parameters_.at (4)));

		ISH_->ShowBanList (channel, mask, opts.Nick_, time);
	}

	void ServerResponseManager::GotBanListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowBanListEnd (opts.Message_);
	}

	void ServerResponseManager::GotExceptList (const IrcMessageOptions& opts)
	{
		const int count = opts.Parameters_.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;

		if (count > 2)
		{
			channel = QString::fromStdString (opts.Parameters_.at (1));
			mask = QString::fromStdString (opts.Parameters_.at (2));
		}

		if (count > 3)
		{
			QString name = QString::fromStdString (opts.Parameters_.at (3));
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromSecsSinceEpoch (std::stoi (opts.Parameters_.at (4)));

		ISH_->ShowExceptList (channel, mask, opts.Nick_, time);
	}

	void ServerResponseManager::GotExceptListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowExceptListEnd (opts.Message_);
	}

	void ServerResponseManager::GotInviteList (const IrcMessageOptions& opts)
	{
		const int count = opts.Parameters_.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;

		if (count > 2)
		{
			channel = QString::fromStdString (opts.Parameters_.at (1));
			mask = QString::fromStdString (opts.Parameters_.at (2));
		}

		if (count > 3)
		{
			QString name = QString::fromStdString (opts.Parameters_.at (3));
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromSecsSinceEpoch (std::stoi (opts.Parameters_.at (4)));

		ISH_->ShowInviteList (channel, mask, opts.Nick_, time);
	}

	void ServerResponseManager::GotInviteListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowInviteListEnd (opts.Message_);
	}

	void ServerResponseManager::GotServerInfo (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("myinfo",
				Util::Map (opts.Parameters_, &QString::fromStdString).join (" "));

		QString ircServer = QString::fromStdString (opts.Parameters_.at (2));
		IrcServer server;
		auto serversKeys = MatchString2Server_.keys ();
		auto it = std::find_if (serversKeys.begin (), serversKeys.end (),
				[&ircServer] (const auto& key) { return ircServer.contains (key, Qt::CaseInsensitive); });

		if (it == serversKeys.end ())
			return;

		server = MatchString2Server_ [*it];
		ISH_->SetIrcServerInfo (server, ircServer);

		switch (server)
		{
		case IrcServer::UnknownServer:
			break;
		case IrcServer::UnrealIRCD:
			Command2Action_ ["307"] = [this] (const IrcMessageOptions& opts)
				{
					WhoIsMessage msg;
					msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
					msg.IsRegistered_ = opts.Message_;
					ISH_->ShowWhoIsReply (msg);
				};
			Command2Action_ ["310"] = [this] (const IrcMessageOptions& opts)
				{
					WhoIsMessage msg;
					msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
					msg.IsHelpOp_ = opts.Message_;
					ISH_->ShowWhoIsReply (msg);
				};
			Command2Action_ ["320"] = [this] (const IrcMessageOptions& opts)
				{
					WhoIsMessage msg;
					msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
					msg.Mail_ = opts.Message_;
					ISH_->ShowWhoIsReply (msg);
				};
			Command2Action_ ["378"] = [this] (const IrcMessageOptions& opts)
				{
					WhoIsMessage msg;
					msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
					msg.ConnectedFrom_ = opts.Message_;
					ISH_->ShowWhoIsReply (msg);
				};
			break;
		}
	}

	void ServerResponseManager::GotWhoIsAccount (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.LoggedName_ = QString::fromStdString (opts.Parameters_.at (2));
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsSecure (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.Secure_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotChannelUrl (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->GotChannelUrl (QString::fromStdString (opts.Parameters_.at (1)),
				opts.Message_);
	}

	void ServerResponseManager::GotTopicWhoTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		ISH_->GotTopicWhoTime (QString::fromStdString (opts.Parameters_.at (1)),
				QString::fromStdString (opts.Parameters_.at (2)),
				std::stoll (opts.Parameters_.at (3)));
	}


}
}
}

