/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "rssparser.h"
#include "channel.h"

namespace LC
{
namespace Aggregator
{
	class RSS20Parser : public RSSParser
	{
		RSS20Parser () = default;
	public:
		static RSS20Parser& Instance ();
		bool CouldParse (const QDomDocument&) const override;
	private:
		channels_container_t Parse (const QDomDocument&, const IDType_t&) const override;
		Item_ptr ParseItem (const QDomElement&, const IDType_t&) const;
	};
}
}
