/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_SERVERRESPONCEMANAGER_H
#define LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_SERVERRESPONCEMANAGER_H

#include <boost/function.hpp>
#include <string>
#include <QObject>
#include <QHash>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerHandler;

	class ServerResponceManager : public QObject
	{
		Q_OBJECT

		IrcServerHandler *ISH_;
		QHash<QString, boost::function<void (const QString&,
				const QList<std::string>&, const QString&)> > Command2Action_;
	public:
		ServerResponceManager (IrcServerHandler*);
		void DoAction (const QString&, const QString&, 
				const QList<std::string>&, const QString&);
	private:
		void Init ();
		bool IsCTCPMessage (const QString&);
		void GotJoin (const QString&, const QList<std::string>&, 
				const QString&);
		void GotPart (const QString&, const QList<std::string>&, 
				const QString&);
		void GotQuit (const QString&, const QList<std::string>&, 
				const QString&);
		void GotPrivMsg (const QString&, const QList<std::string>&, 
				const QString&);
		void GotNoticeMsg (const QString&, const QList<std::string>&, 
				const QString&);
		void GotNick (const QString&, const QList<std::string>&, 
				const QString&);
		void GotPing (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTopic (const QString&, const QList<std::string>&, 
				const QString&);
		void GotKick (const QString&, const QList<std::string>&, 
				const QString&);
		void GotInvitation (const QString&, const QList<std::string>&, 
				const QString&);
		void ShowInviteMessage (const QString&, const QList<std::string>&, 
				const QString&);
		void GotCTCPReply (const QString&, const QList<std::string>&, 
				const QString&);
		void GotCTCPRequestResult (const QString&, const QList<std::string>&, 
				const QString&);
		void GotNames (const QString&, const QList<std::string>&, 
				const QString&);
		void GotEndOfNames (const QString&, const QList<std::string>&, 
				const QString&);
		void GotAwayReply (const QString&, const QList<std::string>&, 
				const QString&);
		void GotSetAway (const QString&, const QList<std::string>&, 
				const QString&);
		void GotUserHost (const QString&, const QList<std::string>&, 
				const QString&);
		void GotIson (const QString&, const QList<std::string>&, 
				const QString&);
		void GotWhoIsUser (const QString&, const QList<std::string>&, 
				const QString&);
		void GotWhoIsServer (const QString&, const QList<std::string>&, 
				const QString&);
		void GotWhoIsOperator (const QString&, const QList<std::string>&, 
				const QString&);
		void GotWhoIsIdle (const QString&, const QList<std::string>&,
				const QString&);
		void GotEndOfWhoIs (const QString&, const QList<std::string>&,
				const QString&);
		void GotWhoIsChannels (const QString&, const QList<std::string>&,
				const QString&);
		void GotWhoWas (const QString&, const QList<std::string>&, 
				const QString&);
		void GotEndOfWhoWas (const QString&, const QList<std::string>&, 
				const QString&);
		void GotWho (const QString&, const QList<std::string>&, 
				const QString&);
		void GotEndOfWho (const QString&, const QList<std::string>&, 
				const QString&);
		void GotSummoning (const QString&, const QList<std::string>&, 
				const QString&);
		void GotVersion (const QString&, const QList<std::string>&, 
				const QString&);
		void GotLinks (const QString&, const QList<std::string>&, 
				const QString&);
		void GotEndOfLinks (const QString&, const QList<std::string>&, 
				const QString&);
		void GotInfo (const QString&, const QList<std::string>&, 
				const QString&);
		void GotEndOfInfo (const QString&, const QList<std::string>&, 
				const QString&);
		void GotMotd (const QString&, const QList<std::string>&, 
				const QString&);
		void GotEndOfMotd (const QString&, const QList<std::string>&, 
				const QString&);
		void GotYoureOper (const QString&, const QList<std::string>&, 
				const QString&);
		void GotRehash (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTime (const QString&, const QList<std::string>&, 
				const QString&);
		void GotLuserOnlyMsg (const QString&, const QList<std::string>&,
				const QString&);
		void GotLuserParamsWithMsg (const QString&, const QList<std::string>&,
				const QString&);
		void GotUsersStart (const QString&, const QList<std::string>&, 
				const QString&);
		void GotUsers (const QString&, const QList<std::string>&, 
				const QString&);
		void GotNoUser (const QString&, const QList<std::string>&, 
				const QString&);
		void GotEndOfUsers (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceLink (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceConnecting (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceHandshake (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceUnknown (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceOperator (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceUser (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceServer (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceService (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceNewType (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceClass (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceLog (const QString&, const QList<std::string>&, 
				const QString&);
		void GotTraceEnd (const QString&, const QList<std::string>&,
				const QString&);
		void GotStatsLinkInfo (const QString&, const QList<std::string>&,
				const QString&);
		void GotStatsCommands (const QString&, const QList<std::string>&,
				const QString&);
		void GotStatsEnd (const QString&, const QList<std::string>&,
				const QString&);
		void GotStatsUptime (const QString&, const QList<std::string>&,
				const QString&);
		void GotStatsOline (const QString&, const QList<std::string>&,
				const QString&);
		void GotAdmineMe (const QString&, const QList<std::string>&,
				const QString&);
		void GotAdminLoc1 (const QString&, const QList<std::string>&,
				const QString&);
		void GotAdminLoc2 (const QString&, const QList<std::string>&,
				const QString&);
		void GotAdminEmail (const QString&, const QList<std::string>&,
				const QString&);
		void GotTryAgain (const QString&, const QList<std::string>&,
				const QString&);
		void GotISupport (const QString&, const QList<std::string>&,
				const QString&);
		void GotChannelMode (const QString&, const QList<std::string>&,
				const QString&);
		void GotChannelModes (const QString&, const QList<std::string>&,
				const QString&);
		void GotBanList (const QString&, const QList<std::string>&,
				const QString&);
		void GotBanListEnd (const QString&, const QList<std::string>&,
				const QString&);
		void GotExceptList (const QString&, const QList<std::string>&,
				const QString&);
		void GotExceptListEnd (const QString&, const QList<std::string>&,
				const QString&);
		void GotInviteList (const QString&, const QList<std::string>&,
				const QString&);
		void GotInviteListEnd (const QString&, const QList<std::string>&,
				const QString&);
	};
};
};
};

#endif // LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_SERVERRESPONCEMANAGER_H
