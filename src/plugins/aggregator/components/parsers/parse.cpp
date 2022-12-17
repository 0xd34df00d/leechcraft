/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "parse.h"
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "atom.h"
#include "rss.h"
#include "utils.h"

namespace LC::Aggregator::Parsers
{
	namespace
	{
		QString FixItemTitle (QString&& title)
		{
			return std::move (title).trimmed ().simplified ();
		}

		void PostprocessParsed (channels_container_t& channels)
		{
			for (const auto& newChannel : channels)
			{
				if (newChannel->Link_.isEmpty ())
				{
					qWarning () << "detected empty link for"
						<< newChannel->Title_;
					newChannel->Link_ = "about:blank"_qs;
				}
				for (const auto& item : newChannel->Items_)
					item->Title_ = FixItemTitle (std::move (item->Title_));
			}
		}
	}

	std::optional<channels_container_t> TryParse (const QDomDocument& doc, IDType_t feedId)
	{
		static const std::array parsers
		{
			&Atom10,
			&Rss20,
			&Atom03,
			&Rss091,
			&Rss10,
		};

		for (auto parser : parsers)
			if (auto res = parser (doc, feedId))
			{
				PostprocessParsed (*res);
				return res;
			}

		return {};
	}
}
