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

#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	bool operator== (const ChannelOptions& channel1,
			const ChannelOptions& channel2)
	{
		return (channel1.ChannelName_ == channel2.ChannelName_) &&
				(channel1.ChannelPassword_ == channel2.ChannelPassword_)
				&& (channel1.ServerName_ == channel2.ServerName_);
	}

	ChannelModes::ChannelModes ()
	: InviteMode_ (false)
	, ModerateMode_ (false)
	, BlockOutsideMessageMode_ (false)
	, PrivateMode_ (false)
	, SecretMode_ (false)
	, ReOpMode_ (false)
	, OnlyOpChangeTopicMode_ (false)
	, UserLimit_ (qMakePair (false, 0))
	, ChannelKey_ (qMakePair (false, QString ()))
	{
	}

	bool operator== (const NickServIdentify& nsi1,
					const NickServIdentify& nsi2)
	{
		return	(nsi1.Server_ == nsi2.Server_) &&
				(nsi1.Nick_ == nsi2.Nick_) &&
				(nsi1.NickServNick_ == nsi2.NickServNick_) &&
				(nsi1.AuthString_ == nsi2.AuthString_) &&
				(nsi1.AuthMessage_ == nsi2.AuthMessage_);
	}
};
};
};