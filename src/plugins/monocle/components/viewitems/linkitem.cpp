/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linkitem.h"
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPen>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/qtutil.h>
#include "components/services/linkactionexecutor.h"
#include "pagegraphicsitem.h"

namespace LC::Monocle
{
	namespace
	{
		QString CombineTooltips (const QString& action, const QString& link)
		{
			if (action.isEmpty () && link.isEmpty ())
				return {};
			if (action.isEmpty ())
				return link;
			if (link.isEmpty ())
				return action;

			return link + "<hr/>"_qs + action;
		}
	}

	LinkItem::LinkItem (const ILink_ptr& link, QGraphicsItem *parent, LinkExecutionContext& ec)
	: QGraphicsRectItem { parent }
	, ExecutionContext_ { ec }
	, Link_ { link }
	{
		setCursor (Qt::PointingHandCursor);
		setPen (Qt::NoPen);
		setFlag (QGraphicsItem::ItemHasNoContents);

		const auto& actionTooltip = GetLinkActionTooltip (link->GetLinkAction ());
		const auto& linkTooltip = link->GetToolTip ();
		setToolTip (CombineTooltips (actionTooltip, linkTooltip));
	}

	void LinkItem::contextMenuEvent (QGraphicsSceneContextMenuEvent *event)
	{
		QMenu menu;
		AddLinkMenuActions (Link_->GetLinkAction (), menu, ExecutionContext_);
		GetProxyHolder ()->GetIconThemeManager ()->ManageWidget (&menu);
		menu.exec (event->screenPos ());
	}

	void LinkItem::mousePressEvent (QGraphicsSceneMouseEvent *event)
	{
		NonDragFilter_.RecordPressed (event);
	}

	void LinkItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
	{
		if (NonDragFilter_.IsNonDragRelease (event))
			ExecuteLinkAction (Link_->GetLinkAction (), ExecutionContext_);
	}

	void CreateLinksItems (LinkExecutionContext& docTab, IDocument& doc, const QVector<PageGraphicsItem*>& pages)
	{
		for (auto page : pages)
			for (const auto& link : doc.GetPageLinks (page->GetPageNum ()))
			{
				auto item = new LinkItem { link, page, docTab };
				page->RegisterChildRect (item, link->GetArea (),
						[item] (const PageAbsoluteRect& rect) { item->setRect (rect.ToRectF ()); });
			}
	}
}
