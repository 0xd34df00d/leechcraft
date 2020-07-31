/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "rssparser.h"

namespace LC
{
namespace Aggregator
{
	class RSS091Parser : public RSSParser
	{
		RSS091Parser () = default;
	public:
		static RSS091Parser& Instance ();
		bool CouldParse (const QDomDocument&) const override;
	protected:
		channels_container_t Parse (const QDomDocument&, const IDType_t&) const override;
		Item_ptr ParseItem (const QDomElement&, const IDType_t&) const;
	};
}
}
