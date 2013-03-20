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

#include <QWidget>
#include "interfaces/monocle/idocument.h"
#include "ui_thumbswidget.h"

namespace LeechCraft
{
namespace Monocle
{
	class PagesLayoutManager;

	class ThumbsWidget : public QWidget
	{
		Q_OBJECT

		Ui::ThumbsWidget Ui_;
		QGraphicsScene Scene_;

		PagesLayoutManager *LayoutMgr_;

		IDocument_ptr CurrentDoc_;
	public:
		ThumbsWidget (QWidget* = 0);

		void HandleDoc (IDocument_ptr);
	public slots:
		void handleCurrentPage (int);
	signals:
		void pageClicked (int);
	};
}
}
