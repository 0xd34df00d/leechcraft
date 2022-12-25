/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tooltipbuilder.h"

namespace LC::Aggregator
{
	TooltipBuilder::TooltipBuilder (const QString& title)
	: Tooltip_ { "<b>" + title + "</b>" }
	{
	}

	TooltipBuilder& TooltipBuilder::Add (const QString& name, const QString& value)
	{
		if (!value.isEmpty ())
			Tooltip_ += "<br/><b>" + name + "</b>: " + value;
		return *this;
	}

	TooltipBuilder& TooltipBuilder::Add (const QString& text, int numeric)
	{
		if (numeric > 0)
			Tooltip_ += "<br/>" + text;
		return *this;
	}

	namespace
	{
		void RemoveTag (const QString& name, QString& str)
		{
			int startPos = 0;
			while ((startPos = str.indexOf ("<" + name, startPos, Qt::CaseInsensitive)) >= 0)
			{
				const int end = str.indexOf ('>', startPos);
				if (end < 0)
					return;

				str.remove (startPos, end - startPos + 1);
			}
		}

		void RemovePair (const QString& name, QString& str)
		{
			RemoveTag (name, str);
			RemoveTag ('/' + name, str);
		}
	}


	TooltipBuilder & TooltipBuilder::AddHtml (QString html)
	{
		const int maxSize = 1000;
		RemoveTag ("img", html);
		RemovePair ("font", html);
		RemovePair ("span", html);
		RemovePair ("p", html);
		RemovePair ("div", html);
		for (auto i : { 1, 2, 3, 4, 5, 6 })
			RemovePair ("h" + QString::number (i), html);

		Tooltip_ += "<br />" + html.left (maxSize);
		if (html.size () > maxSize)
			Tooltip_ += "...";

		return *this;
	}

	TooltipBuilder& TooltipBuilder::operator+= (const QString& text)
	{
		Tooltip_ += "<br/>" + text;
		return *this;
	}

	QString TooltipBuilder::GetTooltip() const
	{
		return Tooltip_;
	}
}
