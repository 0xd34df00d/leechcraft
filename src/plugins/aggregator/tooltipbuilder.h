/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC::Aggregator
{
	class TooltipBuilder
	{
		QString Tooltip_;
	public:
		explicit TooltipBuilder (const QString& title);

		TooltipBuilder& Add (const QString& name, const QString& value);
		TooltipBuilder& Add (const QString& text, int numeric);
		TooltipBuilder& AddHtml (QString html);

		TooltipBuilder& operator+= (const QString& text);

		QString GetTooltip () const;
	};
}
