/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef VIEW_H
#define VIEW_H
#include <QTableView> 
class QHeaderView;

/** @class View view.h
 * @brief Represents data on the screen.
 * @author 0xd34df00d
 * This class represents the download list on the screen, nothing
 * more.
 */
class View : public QTableView
{
	Q_OBJECT
public:
	View (QWidget *parent = 0);
	void DoResizes (int size);

	enum UserInputActions
	{
		CellClick
		, CellDoubleClick
	};
};

#endif

