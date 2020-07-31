/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsScene>

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	class VisScene : public QGraphicsScene
	{
		Q_OBJECT
	public:
		VisScene (QObject* = 0);
	protected:
		void drawBackground (QPainter*, const QRectF&);
	signals:
		void redrawing ();
	};
}
}
}
