/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include <iostream>
#include <QHeaderView>
#include "core.h"
#include "view.h"

/** @brief Default constructor.
 * Initializes the object and does some UI-related things.
 *
 * @param parent pointer to the parent QWidget.
 */
View::View (QWidget *parent)
: QTableView (parent)
{
	QHeaderView *hh = horizontalHeader ();
	hh->setSortIndicatorShown (false);
	hh->setStretchLastSection (true);
	hh->setMovable (true);
	
	setShowGrid (false);
	setEditTriggers (QAbstractItemView::DoubleClicked);
	setSelectionBehavior (QAbstractItemView::SelectRows);
	setSelectionMode (QAbstractItemView::SingleSelection);
	setAlternatingRowColors (true);
}

/** @brief Resizes the columns.
 * Resizes the columns according to the given size.
 *
 * @param size overall size.
 */
void View::DoResizes (int size)
{
	QHeaderView *hh = horizontalHeader ();
	hh->resizeSection (0, static_cast<int> (static_cast<double> (size) / 2));
	hh->resizeSection (1, static_cast<int> (static_cast<double> (size) / 2));
}

