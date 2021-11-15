/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include "localtypes.h"

class QUrl;

namespace LC::Azoth::Acetamide
{
	struct DecodedUrl
	{
		ServerOptions Server_;
		ChannelOptions Channel_;
		bool ChannelPassword_ = false;
	};

	std::optional<DecodedUrl> DecodeUrl (const QUrl&);
}
