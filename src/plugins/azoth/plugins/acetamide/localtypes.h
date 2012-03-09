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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H

#include <QStringList>
#include <QPair>
#include <QDateTime>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	struct ServerOptions
	{
		QString ServerName_;
		QString ServerEncoding_;
		QString ServerPassword_;
		QString ServerNickName_;
		int ServerPort_;
		bool SSL_;
		bool NickServIdentify_;
	};

	struct ChannelOptions
	{
		QString ServerName_;
		QString ChannelName_;
		QString ChannelPassword_;
	};

	enum ConnectionState
	{
		Connected,
		InProgress,
		NotConnected
	};

	enum ChannelRole
	{
		Participant,
		Voiced,
		HalfOperator,
		Operator,
		Admin,
		Owner
	};

	enum ChannelManagment
	{
		Kick,
		BanByName,
		BanByDomain,
		BanByUserAndDomain,
		KickAndBan
	};

	struct IrcMessageOptions
	{
		QString Nick_;
		QString UserName_;
		QString Host_;
		QString Command_;
		QString Message_;
		QList<std::string> Parameters_;
	};

	struct IrcBookmark
	{
		QString Name_;
		QString ServerName_;
		QString ServerEncoding_;
		QString NickName_;
		QString ChannelName_;
		QString ChannelPassword_;
		int ServerPort_;
		bool SSL_;
		bool AutoJoin_;
	};

	struct ChannelModes
	{
		ChannelModes ();

		bool InviteMode_;
		bool ModerateMode_;
		bool BlockOutsideMessageMode_;
		bool PrivateMode_;
		bool SecretMode_;
		bool ReOpMode_;
		bool OnlyOpChangeTopicMode_;
		QPair<bool, int> UserLimit_;
		QPair<bool, QString> ChannelKey_;
	};

	struct NickServIdentify
	{
		QString Server_;
		QString Nick_;
		QString NickServNick_;
		QString AuthString_;
		QString AuthMessage_;
	};

	struct MainEntryInfo
	{
		QString Nick_;
		QString UserName_;
		QString Host_;
		QString RealName_;
		QString ServerName_;
	};

	struct WhoMessage : MainEntryInfo
	{
		QString Flags_;
		QString Channel_;
		bool IsAway_;
		int Jumps_;
		QString EndString_;
	};

	struct WhoIsMessage : MainEntryInfo
	{
		QStringList Channels_;
		QString IdentifyAs_;
		QString IdleTime_;
		QString AuthTime_;
		QString IrcOperator_;
		QString ServerCountry_;
		QString LoggedName_;
		QString Secure_;
		QString EndString_;
	};

	bool operator== (const ChannelOptions&, const ChannelOptions&);

	bool operator== (const NickServIdentify&, const NickServIdentify&);
};
};
};


#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
