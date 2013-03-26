/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/monocle/idocument.h"
#include <util/gui/findnotification.h>

class QGraphicsRectItem;
class QGraphicsView;
class QGraphicsScene;

namespace LeechCraft
{
namespace Monocle
{
	class PageGraphicsItem;
	class PagesLayoutManager;

	class TextSearchHandler : public QObject
	{
		Q_OBJECT

		QGraphicsView * const View_;
		QGraphicsScene * const Scene_;
		PagesLayoutManager * const LayoutMgr_;

		IDocument_ptr Doc_;
		QList<PageGraphicsItem*> Pages_;

		QString CurrentSearchString_;

		QList<QGraphicsRectItem*> CurrentHighlights_;
		int CurrentRectIndex_;
	public:
		TextSearchHandler (QGraphicsView*, PagesLayoutManager*, QObject* = 0);

		void HandleDoc (IDocument_ptr, const QList<PageGraphicsItem*>&);

		void Search (const QString&, Util::FindNotification::FindFlags);
	private:
		void SelectItem (int);
	};
}
}
