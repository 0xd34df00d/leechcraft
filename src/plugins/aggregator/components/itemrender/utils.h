/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QString>
#include <util/sll/xmlnode.h>

namespace LC::Aggregator
{
	struct TrCtx
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ItemToHtml)
	};

	struct TextColor
	{
		QString Fg_;
		QString Bg_;
	};

	Util::Tag MakeLink (const QString& target, Util::Node contents);
	Util::Tag WithInnerPadding (const TextColor& color, Util::Nodes&& children);
}
