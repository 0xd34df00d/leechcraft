/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QHash>
#include <QMap>
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	class IrcServerHandler;
	struct IrcMessageOptions;

	class ServerResponseManager : public QObject
	{
		IrcServerHandler * const ISH_;
	public:
		explicit ServerResponseManager (IrcServerHandler*);

        void DoAction (const IrcMessageOptions& opts);

		void GotJoin (const IrcMessageOptions& opts);
		void GotPart (const IrcMessageOptions& opts);
		void GotQuit (const IrcMessageOptions& opts);
		void GotPrivMsg (const IrcMessageOptions& opts);
		void GotNoticeMsg (const IrcMessageOptions& opts);
		void GotNick (const IrcMessageOptions& opts);
		void GotPing (const IrcMessageOptions& opts);
		void GotTopic (const IrcMessageOptions& opts);
		void GotKick (const IrcMessageOptions& opts);
		void GotInvitation (const IrcMessageOptions& opts);
		void ShowInviteMessage (const IrcMessageOptions& opts);
		void GotCTCPReply (const IrcMessageOptions& opts);
		void GotCTCPRequestResult (const IrcMessageOptions& opts);
		void GotNames (const IrcMessageOptions& opts);
		void GotEndOfNames (const IrcMessageOptions& opts);
		void GotAwayReply (const IrcMessageOptions& opts);
		void GotSetAway (const IrcMessageOptions& opts);
		void GotUserHost (const IrcMessageOptions& opts);
		void GotIson (const IrcMessageOptions& opts);
		void GotWhoIsUser (const IrcMessageOptions& opts);
		void GotWhoIsServer (const IrcMessageOptions& opts);
		void GotWhoIsOperator (const IrcMessageOptions& opts);
		void GotWhoIsIdle (const IrcMessageOptions& opts);
		void GotEndOfWhoIs (const IrcMessageOptions& opts);
		void GotWhoIsChannels (const IrcMessageOptions& opts);
		void GotWhoWas (const IrcMessageOptions& opts);
		void GotEndOfWhoWas (const IrcMessageOptions& opts);
		void GotWho (const IrcMessageOptions& opts);
		void GotEndOfWho (const IrcMessageOptions& opts);
		void GotSummoning (const IrcMessageOptions& opts);
		void GotVersion (const IrcMessageOptions& opts);
		void GotLinks (const IrcMessageOptions& opts);
		void GotEndOfLinks (const IrcMessageOptions& opts);
		void GotInfo (const IrcMessageOptions& opts);
		void GotEndOfInfo (const IrcMessageOptions& opts);
		void GotMotd (const IrcMessageOptions& opts);
		void GotEndOfMotd (const IrcMessageOptions& opts);
		void GotYoureOper (const IrcMessageOptions& opts);
		void GotRehash (const IrcMessageOptions& opts);
		void GotTime (const IrcMessageOptions& opts);
		void GotLuserOnlyMsg (const IrcMessageOptions& opts);
		void GotLuserParamsWithMsg (const IrcMessageOptions& opts);
		void GotUsersStart (const IrcMessageOptions& opts);
		void GotUsers (const IrcMessageOptions& opts);
		void GotNoUser (const IrcMessageOptions& opts);
		void GotEndOfUsers (const IrcMessageOptions& opts);
		void GotTraceLink (const IrcMessageOptions& opts);
		void GotTraceConnecting (const IrcMessageOptions& opts);
		void GotTraceHandshake (const IrcMessageOptions& opts);
		void GotTraceUnknown (const IrcMessageOptions& opts);
		void GotTraceOperator (const IrcMessageOptions& opts);
		void GotTraceUser (const IrcMessageOptions& opts);
		void GotTraceServer (const IrcMessageOptions& opts);
		void GotTraceService (const IrcMessageOptions& opts);
		void GotTraceNewType (const IrcMessageOptions& opts);
		void GotTraceClass (const IrcMessageOptions& opts);
		void GotTraceLog (const IrcMessageOptions& opts);
		void GotTraceEnd (const IrcMessageOptions& opts);
		void GotStatsLinkInfo (const IrcMessageOptions& opts);
		void GotStatsCommands (const IrcMessageOptions& opts);
		void GotStatsEnd (const IrcMessageOptions& opts);
		void GotStatsUptime (const IrcMessageOptions& opts);
		void GotStatsOline (const IrcMessageOptions& opts);
		void GotAdmineMe (const IrcMessageOptions& opts);
		void GotAdminLoc1 (const IrcMessageOptions& opts);
		void GotAdminLoc2 (const IrcMessageOptions& opts);
		void GotAdminEmail (const IrcMessageOptions& opts);
		void GotTryAgain (const IrcMessageOptions& opts);
		void GotISupport (const IrcMessageOptions& opts);
		void GotChannelMode (const IrcMessageOptions& opts);
		void GotChannelModes (const IrcMessageOptions& opts);
		void GotBanList (const IrcMessageOptions& opts);
		void GotBanListEnd (const IrcMessageOptions& opts);
		void GotExceptList (const IrcMessageOptions& opts);
		void GotExceptListEnd (const IrcMessageOptions& opts);
		void GotInviteList (const IrcMessageOptions& opts);
		void GotInviteListEnd (const IrcMessageOptions& opts);
		void GotServerInfo (const IrcMessageOptions& opts);

		//not from rfc
		void GotWhoIsAccount (const IrcMessageOptions& opts);
		void GotWhoIsSecure (const IrcMessageOptions& opts);
		void GotChannelUrl (const IrcMessageOptions& opts);
		void GotTopicWhoTime (const IrcMessageOptions& opts);
	private:
		bool IsCTCPMessage (const QString&);
	};
}
