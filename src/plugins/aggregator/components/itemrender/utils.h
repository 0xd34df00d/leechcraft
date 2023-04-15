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
#include "xmlnode.h"

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

	Tag MakeLink (const QString& target, Node contents);
	Tag WithInnerPadding (const TextColor& color, Nodes&& children);
}
