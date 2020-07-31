/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/monocle/idocument.h"

class QGraphicsView;
class QGraphicsScene;

namespace LC
{
namespace Monocle
{
	class PageGraphicsItem;

	class LinksManager : public QObject
	{
		Q_OBJECT

		QGraphicsView * const View_;
		QGraphicsScene * const Scene_;
	public:
		LinksManager (QGraphicsView*, QObject* = 0);

		void HandleDoc (IDocument_ptr, const QList<PageGraphicsItem*>&);
	};
}
}
