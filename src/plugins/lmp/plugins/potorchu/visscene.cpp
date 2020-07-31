/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "visscene.h"
#include <QPainter>

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	VisScene::VisScene (QObject *parent)
	: QGraphicsScene { parent }
	{
	}

	void VisScene::drawBackground (QPainter *p, const QRectF&)
	{
		p->beginNativePainting ();
		emit redrawing ();
		p->endNativePainting ();
	}
}
}
}
