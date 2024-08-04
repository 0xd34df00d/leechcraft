/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagescontextmenuhandler.h"
#include <QContextMenuEvent>
#include <QGraphicsView>
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/lambdaeventfilter.h>
#include "components/actions/rotatemenu.h"
#include "pagegraphicsitem.h"
#include "pageslayoutmanager.h"

namespace LC::Monocle
{
	void HandlePagesContextMenus (QGraphicsView& view, PagesLayoutManager& layoutManager)
	{
		view.installEventFilter (Util::MakeLambdaEventFilter ([&] (QContextMenuEvent *event)
				{
					const auto page = dynamic_cast<PageGraphicsItem*> (view.itemAt (event->pos ()));
					if (!page)
						return false;

					const auto pageNum = page->GetPageNum ();

					auto rotateMenu = CreateRotateMenu (InitAngle { layoutManager.GetPageRotation (pageNum) },
							std::bind_front (&PagesLayoutManager::SetPageRotation, &layoutManager, pageNum));
					GetProxyHolder ()->GetIconThemeManager ()->ManageWidget (rotateMenu.get ());
					rotateMenu->exec (event->globalPos ());

					return true;
				},
				view));
	}
}
