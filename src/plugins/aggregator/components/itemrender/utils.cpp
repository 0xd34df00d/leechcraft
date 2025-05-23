/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utils.h"
#include <util/sll/qtutil.h>
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	Util::Tag MakeLink (const QString& target, Util::Node contents)
	{
		Util::TagAttrs attrs { { "href"_qba, target } };
		if (XmlSettingsManager::Instance ().property ("AlwaysUseExternalBrowser").toBool ())
			attrs.push_back ({ "target"_qba, "blank"_qs });

		return { .Name_ = "a", .Attrs_ = std::move (attrs), .Children_ = { std::move (contents) } };
	}

	Util::Tag WithInnerPadding (const TextColor& color, Util::Nodes&& children)
	{
		auto blockStyle = R"(
					background: %1;
					color: %2;
					border: 1px solid #333333;
					padding: 0.2em 2em;
					-webkit-border-radius: 1em;
					)"_qs
				.arg (color.Bg_, color.Fg_);
		return
		{
			.Name_ = "div"_qba,
			.Attrs_ = { { "style"_qba, std::move (blockStyle) } },
			.Children_ = std::move (children),
		};
	}

}
