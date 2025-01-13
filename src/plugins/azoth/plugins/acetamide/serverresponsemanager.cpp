/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverresponsemanager.h"
#include <util/sll/qtutil.h>
#include <util/sll/statichash.h>
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include "ircserverhandler.h"
#include "ircaccount.h"
#include "localtypes.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth::Acetamide
{
	namespace
	{
		template<typename T>
		using CmdImpl_t = void (T::*) (const IrcMessageOptions&);

		using Util::KVPair;

		constexpr auto SRMHash = Util::MakeStringHash<CmdImpl_t<ServerResponseManager>> (
					KVPair { "join", &ServerResponseManager::GotJoin },
					KVPair { "part", &ServerResponseManager::GotPart },
					KVPair { "quit", &ServerResponseManager::GotQuit },
					KVPair { "privmsg", &ServerResponseManager::GotPrivMsg },
					KVPair { "notice", &ServerResponseManager::GotNoticeMsg },
					KVPair { "nick", &ServerResponseManager::GotNick },
					KVPair { "ping", &ServerResponseManager::GotPing },
					KVPair { "topic", &ServerResponseManager::GotTopic },
					KVPair { "kick", &ServerResponseManager::GotKick },
					KVPair { "invite", &ServerResponseManager::GotInvitation },
					KVPair { "ctcp_rpl", &ServerResponseManager::GotCTCPReply },
					KVPair { "ctcp_rqst", &ServerResponseManager::GotCTCPRequestResult },
					KVPair { "mode", &ServerResponseManager::GotChannelMode },
					KVPair { "331", &ServerResponseManager::GotTopic },
					KVPair { "332", &ServerResponseManager::GotTopic },
					KVPair { "341", &ServerResponseManager::ShowInviteMessage },
					KVPair { "353", &ServerResponseManager::GotNames },
					KVPair { "366", &ServerResponseManager::GotEndOfNames },
					KVPair { "301", &ServerResponseManager::GotAwayReply },
					KVPair { "305", &ServerResponseManager::GotSetAway },
					KVPair { "306", &ServerResponseManager::GotSetAway },
					KVPair { "302", &ServerResponseManager::GotUserHost },
					KVPair { "303", &ServerResponseManager::GotIson },
					KVPair { "311", &ServerResponseManager::GotWhoIsUser },
					KVPair { "312", &ServerResponseManager::GotWhoIsServer },
					KVPair { "313", &ServerResponseManager::GotWhoIsOperator },
					KVPair { "317", &ServerResponseManager::GotWhoIsIdle },
					KVPair { "318", &ServerResponseManager::GotEndOfWhoIs },
					KVPair { "319", &ServerResponseManager::GotWhoIsChannels },
					KVPair { "314", &ServerResponseManager::GotWhoWas },
					KVPair { "369", &ServerResponseManager::GotEndOfWhoWas },
					KVPair { "352", &ServerResponseManager::GotWho },
					KVPair { "315", &ServerResponseManager::GotEndOfWho },
					KVPair { "342", &ServerResponseManager::GotSummoning },
					KVPair { "351", &ServerResponseManager::GotVersion },
					KVPair { "364", &ServerResponseManager::GotLinks },
					KVPair { "365", &ServerResponseManager::GotEndOfLinks },
					KVPair { "371", &ServerResponseManager::GotInfo },
					KVPair { "374", &ServerResponseManager::GotEndOfInfo },
					KVPair { "372", &ServerResponseManager::GotMotd },
					KVPair { "375", &ServerResponseManager::GotMotd },
					KVPair { "376", &ServerResponseManager::GotEndOfMotd },
					KVPair { "422", &ServerResponseManager::GotMotd },
					KVPair { "381", &ServerResponseManager::GotYoureOper },
					KVPair { "382", &ServerResponseManager::GotRehash },
					KVPair { "391", &ServerResponseManager::GotTime },
					KVPair { "251", &ServerResponseManager::GotLuserOnlyMsg },
					KVPair { "252", &ServerResponseManager::GotLuserParamsWithMsg },
					KVPair { "253", &ServerResponseManager::GotLuserParamsWithMsg },
					KVPair { "254", &ServerResponseManager::GotLuserParamsWithMsg },
					KVPair { "255", &ServerResponseManager::GotLuserOnlyMsg },
					KVPair { "392", &ServerResponseManager::GotUsersStart },
					KVPair { "393", &ServerResponseManager::GotUsers },
					KVPair { "395", &ServerResponseManager::GotNoUser },
					KVPair { "394", &ServerResponseManager::GotEndOfUsers },
					KVPair { "200", &ServerResponseManager::GotTraceLink },
					KVPair { "201", &ServerResponseManager::GotTraceConnecting },
					KVPair { "202", &ServerResponseManager::GotTraceHandshake },
					KVPair { "203", &ServerResponseManager::GotTraceUnknown },
					KVPair { "204", &ServerResponseManager::GotTraceOperator },
					KVPair { "205", &ServerResponseManager::GotTraceUser },
					KVPair { "206", &ServerResponseManager::GotTraceServer },
					KVPair { "207", &ServerResponseManager::GotTraceService },
					KVPair { "208", &ServerResponseManager::GotTraceNewType },
					KVPair { "209", &ServerResponseManager::GotTraceClass },
					KVPair { "261", &ServerResponseManager::GotTraceLog },
					KVPair { "262", &ServerResponseManager::GotTraceEnd },
					KVPair { "211", &ServerResponseManager::GotStatsLinkInfo },
					KVPair { "212", &ServerResponseManager::GotStatsCommands },
					KVPair { "219", &ServerResponseManager::GotStatsEnd },
					KVPair { "242", &ServerResponseManager::GotStatsUptime },
					KVPair { "243", &ServerResponseManager::GotStatsOline },
					KVPair { "256", &ServerResponseManager::GotAdmineMe },
					KVPair { "257", &ServerResponseManager::GotAdminLoc1 },
					KVPair { "258", &ServerResponseManager::GotAdminLoc2 },
					KVPair { "259", &ServerResponseManager::GotAdminEmail },
					KVPair { "263", &ServerResponseManager::GotTryAgain },
					KVPair { "005", &ServerResponseManager::GotISupport },
					KVPair { "367", &ServerResponseManager::GotBanList },
					KVPair { "368", &ServerResponseManager::GotBanListEnd },
					KVPair { "348", &ServerResponseManager::GotExceptList },
					KVPair { "349", &ServerResponseManager::GotExceptListEnd },
					KVPair { "346", &ServerResponseManager::GotInviteList },
					KVPair { "347", &ServerResponseManager::GotInviteListEnd },
					KVPair { "324", &ServerResponseManager::GotChannelModes },

					//not from rfc
					KVPair { "330", &ServerResponseManager::GotWhoIsAccount },
					KVPair { "671", &ServerResponseManager::GotWhoIsSecure },
					KVPair { "328", &ServerResponseManager::GotChannelUrl },
					KVPair { "333", &ServerResponseManager::GotTopicWhoTime },
					KVPair { "004", &ServerResponseManager::GotServerInfo }
				);

		constexpr auto ISHHash = Util::MakeStringHash<CmdImpl_t<IrcServerHandler>> (
					KVPair { "321", &IrcServerHandler::GotChannelsListBegin },
					KVPair { "322", &IrcServerHandler::GotChannelsList },
					KVPair { "323", &IrcServerHandler::GotChannelsListEnd }
				);

		using CustomCommand_t = void (*) (IrcServerHandler&, const IrcMessageOptions&);

		constexpr auto UnrealCustoms = Util::MakeStringHash<CustomCommand_t> (
					KVPair
					{
						"307",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = opts.Parameters_ [1];
							msg.IsRegistered_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					},
					KVPair
					{
						"310",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = opts.Parameters_ [1];
							msg.IsHelpOp_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					},
					KVPair
					{
						"320",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = opts.Parameters_ [1];
							msg.Mail_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					},
					KVPair
					{
						"378",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = opts.Parameters_ [1];
							msg.ConnectedFrom_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					}
				);

		constexpr auto MatchString2Server = std::to_array<std::pair<QStringView, IrcServer>> ({
					{ u"unreal", IrcServer::UnrealIRCD }
				});
	}

	ServerResponseManager::ServerResponseManager (IrcServerHandler *ish)
	: ISH_ { ish }
	{
	}

	namespace
	{
		bool IsCTCPMessage (const QString& msg)
		{
			return msg.startsWith ('\001') && msg.endsWith ('\001');
		}
	}

	void ServerResponseManager::DoAction (const IrcMessageOptions& opts)
	{
		auto cmdUtf8 = opts.Command_.toUtf8 ();
		if (cmdUtf8 == "privmsg" && IsCTCPMessage (opts.Message_))
			cmdUtf8 = "ctcp_rpl";
		else if (cmdUtf8 == "notice" && IsCTCPMessage (opts.Message_))
			cmdUtf8 = "ctcp_rqst";

		switch (ISH_->GetServerOptions ().IrcServer_)
		{
		case IrcServer::UnknownServer:
			break;
		case IrcServer::UnrealIRCD:
			if (const auto actor = UnrealCustoms (Util::AsStringView (cmdUtf8)))
			{
				actor (*ISH_, opts);
				return;
			}
			break;
		}

		if (const auto actor = SRMHash (Util::AsStringView (cmdUtf8)))
			(this->*actor) (opts);
		else if (const auto actor = ISHHash (Util::AsStringView (cmdUtf8)))
			(ISH_->*actor) (opts);
		else
			ISH_->ShowAnswer (opts.Command_.toUtf8 (), opts.Message_);
	}

	void ServerResponseManager::GotJoin (const IrcMessageOptions& opts)
	{
		const auto& channel = opts.Message_.isEmpty () ?
				opts.Parameters_.last () :
				opts.Message_;

		if (opts.Nick_ == ISH_->GetNickName ())
		{
			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ServerName_ = ISH_->GetServerOptions ().ServerName_.toLower ();
			ISH_->JoinedChannel (co);
		}
		else
			ISH_->JoinParticipant (opts.Nick_, channel, opts.UserName_, opts.Host_);
	}

	void ServerResponseManager::GotPart (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

        const auto& channel = opts.Parameters_.first ();
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

		const auto& target = opts.Parameters_.first ();
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
		ISH_->GotTopic (opts.Parameters_.last (), opts.Message_);
	}

	void ServerResponseManager::GotKick (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& channel = opts.Parameters_.first ();
		const auto& target = opts.Parameters_.last ();
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

		ISH_->ShowAnswer ("invite",
				tr ("You invite %1 to channel %2")
					.arg (opts.Parameters_.at (1),
						  opts.Parameters_.at (2)));
	}

	void ServerResponseManager::GotCTCPReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty () || opts.Message_.isEmpty ())
			return;

		const auto& ctcpArgs = QStringView { opts.Message_ }.mid (1, opts.Message_.length () - 2);
		if (ctcpArgs.isEmpty ())
			return;

		const auto& [commandStr, commandArg] = Util::BreakAt (ctcpArgs, ' ');
		const auto& command = commandStr.toUtf8 ().toUpper ();

		const auto lcVer = [] { return GetProxyHolder ()->GetVersion (); };
		const auto fullVersion = [&] { return u"LeechCraft %1 (Acetamide 2.0) - https://leechcraft.org"_qsv.arg (lcVer ()); };

		const auto sendReply = [&] (const QString& body)
		{
			ISH_->CTCPReply (opts.Nick_,
					'\001' + command + ' ' + body + '\001',
					tr ("Received request %1 from %2, sending response")
							.arg (command, opts.Nick_));
		};

		if (command == "VERSION")
			sendReply (u"LeechCraft %1 (Acetamide 2.0)"_qsv.arg (lcVer ()));
		else if (command == "PING")
			sendReply (QString::number (QDateTime::currentDateTime ().toSecsSinceEpoch ()));
		else if (command == "TIME")
			sendReply (QDateTime::currentDateTime ().toString (u"ddd MMM dd hh:mm:ss yyyy"_qsv));
		else if (command == "SOURCE")
			sendReply (fullVersion ());
		else if (command == "CLIENTINFO")
			sendReply (fullVersion () + " - Supported tags: VERSION PING TIME SOURCE CLIENTINFO");
		else if (command == "ACTION" && !commandArg.isEmpty ())
		{
			auto msg = "/me "_qs;
			msg += commandArg;
			ISH_->IncomingMessage (opts.Nick_,
					opts.Parameters_.last (),
					msg);
		}
	}

	void ServerResponseManager::GotCTCPRequestResult (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.first () != ISH_->GetNickName ())
			return;

		if (opts.Message_.isEmpty ())
			return;

		const auto& ctcpArg = QStringView { opts.Message_ }.mid (1, opts.Message_.length () - 2);
		if (ctcpArg.isEmpty ())
			return;

		const auto& [command, reply] = Util::BreakAt (ctcpArg, ' ');

		ISH_->CTCPRequestResult (tr ("Received answer CTCP-%1 from %2: %3")
				.arg (command, opts.Nick_, reply));
	}

	void ServerResponseManager::GotNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& channel = opts.Parameters_.last ();
		const auto& participants = opts.Message_.split (' ');
		ISH_->GotNames (channel, participants);
	}

	void ServerResponseManager::GotEndOfNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& channel = opts.Parameters_.last ();
		ISH_->GotEndOfNames (channel);
	}

	void ServerResponseManager::GotAwayReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& target = opts.Parameters_.last ();
		ISH_->IncomingMessage (target,
				target,
				u"[AWAY] %1 :%2"_qsv.arg (target, opts.Message_),
				IMessage::Type::StatusMessage);
	}

	void ServerResponseManager::GotSetAway (const IrcMessageOptions& opts)
	{
		constexpr auto NotAway = 305;
		constexpr auto Away = 306;
		switch (opts.Command_.toInt ())
		{
		case NotAway:
			ISH_->ChangeAway (false);
			break;
		case Away:
			ISH_->ChangeAway (true, opts.Message_);
			break;
		}

		ISH_->ShowAnswer ("away", opts.Message_, true, IMessage::Type::StatusMessage);
	}

	void ServerResponseManager::GotUserHost (const IrcMessageOptions& opts)
	{
		for (const auto& str : QStringView { opts.Message_ }.split (' '))
		{
			const auto& [user, host] = Util::BreakAt (str, '=');
			ISH_->ShowUserHost (user.toString (), host.toString ());
		}
	}

	void ServerResponseManager::GotIson (const IrcMessageOptions& opts)
	{
		for (const auto& str : QStringView { opts.Message_ }.split (' '))
			ISH_->ShowIsUserOnServer (str.toString ());
	}

	void ServerResponseManager::GotWhoIsUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.UserName_ = opts.Parameters_.at (2);
		msg.Host_ = opts.Parameters_.at (3);
		msg.RealName_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.ServerName_ = opts.Parameters_.at (2);
		msg.ServerCountry_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.IrcOperator_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsIdle (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.IdleTime_ = Util::MakeTimeFromLong (opts.Parameters_.at (2).toInt ());
		msg.AuthTime_ = QDateTime::fromSecsSinceEpoch (opts.Parameters_.at (3).toInt ()).toString (Qt::TextDate);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotEndOfWhoIs (const IrcMessageOptions& opts)
	{
		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg, true);
	}

	void ServerResponseManager::GotWhoIsChannels (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.Channels_ = opts.Message_.split (' ', Qt::SkipEmptyParts);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoWas (const IrcMessageOptions& opts)
	{
		ISH_->ShowWhoWasReply (opts.Parameters_.at (1) +
				" - " + opts.Parameters_.at (2) + "@"
				+ opts.Parameters_.at (3) +
				" (" + opts.Message_ + ")");
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
		msg.Channel_ = opts.Parameters_.at (1);
		msg.UserName_ = opts.Parameters_.at (2);
		msg.Host_ = opts.Parameters_.at (3);
		msg.ServerName_ = opts.Parameters_.at (4);
		msg.Nick_ = opts.Parameters_.at (5);

		const auto& [realName, jumps] = Util::BreakAt (QStringView { opts.Message_ }, ' ');
		msg.RealName_ = realName.toString ();
		msg.Jumps_ = jumps.toInt ();

		msg.Flags_ = opts.Parameters_.at (6);
		if (msg.Flags_.at (0) == 'H')
			msg.IsAway_ = false;
		else if (msg.Flags_.at (0) == 'G')
			msg.IsAway_ = true;
		ISH_->ShowWhoReply (msg);
	}

	void ServerResponseManager::GotEndOfWho (const IrcMessageOptions& opts)
	{
		WhoMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoReply (msg, true);
	}

	void ServerResponseManager::GotSummoning (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->ShowAnswer ("summon", opts.Parameters_.at (1) + tr (" summoning to IRC"));
	}

	namespace
	{
		QString BuildParamsStr (const QStringList& params)
		{
			return params.join (' ');
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

		ISH_->ShowAnswer ("rehash", opts.Parameters_.last () + " :" + opts.Message_);
	}

	void ServerResponseManager::GotTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("time", opts.Parameters_.last () + " :" + opts.Message_);
	}

	void ServerResponseManager::GotLuserOnlyMsg (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("luser", opts.Message_);
	}

	void ServerResponseManager::GotLuserParamsWithMsg (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("luser", opts.Parameters_.last () + ":" + opts.Message_);
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

		ISH_->ShowTraceReply (opts.Parameters_.last () + " " + opts.Message_, true);
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

		ISH_->ShowStatsReply (opts.Parameters_.last () + " " + opts.Message_, true);
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

		ISH_->ShowAnswer ("admin", opts.Parameters_.last () + ":" + opts.Message_);
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

		ISH_->ShowAnswer ("error", opts.Parameters_.last () + ":" + opts.Message_);
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
				opts.Parameters_.first () == ISH_->GetNickName ())
		{
			ISH_->ParseUserMode (opts.Parameters_.first (), opts.Message_);
			return;
		}

		const auto& channel = opts.Parameters_.first ();

		if (opts.Parameters_.count () == 2)
			ISH_->ParseChanMode (channel, opts.Parameters_.at (1));
		else if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					opts.Parameters_.at (1),
					opts.Parameters_.at (2));
	}

	void ServerResponseManager::GotChannelModes (const IrcMessageOptions& opts)
	{
		const auto& channel = opts.Parameters_.at (1);

		if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					opts.Parameters_.at (2));
		else if (opts.Parameters_.count () == 4)
			ISH_->ParseChanMode (channel,
					opts.Parameters_.at (2),
					opts.Parameters_.at (3));
	}

	namespace
	{
		void ShowList (const IrcMessageOptions& opts,
				IrcServerHandler& ish,
				void (IrcServerHandler::*handler) (const QString&, const QString&, const QString&, const QDateTime&))
		{
			const auto& channel = opts.Parameters_.value (1);
			const auto& mask = opts.Parameters_.value (2);

			QDateTime time;
			if (const auto& timeStr = opts.Parameters_.value (4);
					!timeStr.isEmpty ())
				time = QDateTime::fromSecsSinceEpoch (timeStr.toInt ());

			(ish.*handler) (channel, mask, opts.Nick_, time);
		}
	}

	void ServerResponseManager::GotBanList (const IrcMessageOptions& opts)
	{
		ShowList (opts, *ISH_, &IrcServerHandler::ShowBanList);
	}

	void ServerResponseManager::GotBanListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowBanListEnd (opts.Message_);
	}

	void ServerResponseManager::GotExceptList (const IrcMessageOptions& opts)
	{
		ShowList (opts, *ISH_, &IrcServerHandler::ShowExceptList);
	}

	void ServerResponseManager::GotExceptListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowExceptListEnd (opts.Message_);
	}

	void ServerResponseManager::GotInviteList (const IrcMessageOptions& opts)
	{
		ShowList (opts, *ISH_, &IrcServerHandler::ShowInviteList);
	}

	void ServerResponseManager::GotInviteListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowInviteListEnd (opts.Message_);
	}

	void ServerResponseManager::GotServerInfo (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("myinfo", opts.Parameters_.join (' '));

		const auto& ircServer = opts.Parameters_.at (2);
		const auto it = std::find_if (MatchString2Server.begin (), MatchString2Server.end (),
				[&ircServer] (const auto& key) { return ircServer.contains (key.first, Qt::CaseInsensitive); });
		const auto server = it == MatchString2Server.end () ? IrcServer::UnknownServer : it->second;
		ISH_->SetIrcServerInfo (server, ircServer);
	}

	void ServerResponseManager::GotWhoIsAccount (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.LoggedName_ = opts.Parameters_.at (2);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsSecure (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = opts.Parameters_.at (1);
		msg.Secure_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotChannelUrl (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->GotChannelUrl (opts.Parameters_.at (1), opts.Message_);
	}

	void ServerResponseManager::GotTopicWhoTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		ISH_->GotTopicWhoTime (opts.Parameters_.at (1),
				opts.Parameters_.at (2),
				opts.Parameters_.at (3).toLongLong ());
	}


}
