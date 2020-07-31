/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "halfpacket.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	HalfPacket::operator Packet ()
	{
		Header_.DataLength_ = Data_.size ();
		return { Header_.Seq_, Header_.Serialize () + Data_ };
	}
}
}
}
}
