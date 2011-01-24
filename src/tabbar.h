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

#ifndef TABBAR_H
#define TABBAR_H
#include <QTabBar>

namespace LeechCraft
{
	class TabBar : public QTabBar
	{
		Q_OBJECT
	public:
		TabBar (QWidget* = 0);
	protected:
		virtual void tabInserted (int);
		virtual void tabRemoved (int);
	signals:
		void tabWasInserted (int);
		void tabWasRemoved (int);
	};
};

#endif

