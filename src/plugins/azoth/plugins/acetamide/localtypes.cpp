/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localtypes.h"

namespace LC
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

	bool operator== (const NickServIdentify& nsi1,
					const NickServIdentify& nsi2)
	{
		return	(nsi1.Server_ == nsi2.Server_) &&
				(nsi1.Nick_ == nsi2.Nick_) &&
				(nsi1.NickServNick_ == nsi2.NickServNick_) &&
				(nsi1.AuthString_ == nsi2.AuthString_) &&
				(nsi1.AuthMessage_ == nsi2.AuthMessage_);
	}
}
}
}
