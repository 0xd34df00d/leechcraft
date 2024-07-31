/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsRectItem>
#include "interfaces/monocle/ilink.h"
#include "nondragclickfilter.h"

namespace LC::Monocle
{
	class IDocument;
	class PageGraphicsItem;

	struct LinkExecutionContext;

	class LinkItem : public QGraphicsRectItem
	{
		LinkExecutionContext& ExecutionContext_;
		const ILink_ptr Link_;

		NonDragClickFilter NonDragFilter_;
	public:
		explicit LinkItem (const ILink_ptr&, QGraphicsItem*, LinkExecutionContext&);
	protected:
		void contextMenuEvent (QGraphicsSceneContextMenuEvent*) override;
		void mousePressEvent (QGraphicsSceneMouseEvent*) override;
		void mouseReleaseEvent (QGraphicsSceneMouseEvent*) override;
	};

	void CreateLinksItems (LinkExecutionContext&, IDocument&, const QVector<PageGraphicsItem*>&);
}
