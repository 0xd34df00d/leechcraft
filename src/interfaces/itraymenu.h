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

#ifndef INTERFACES_ITRAYMENU_H
#define INTERFACES_ITRAYMENU_H
#include <QtPlugin>

class QAction;
class QMenu;

/** @brief Interface for adding actions and menus into tray icon menu.
 *
 * If your plugin wants to add custom actions and menus into LeechCraft's
 * tray menu icon, it should implement this interface. Actions and menus
 * are to be returned by GetTrayActions() and GetTrayMenus()
 * respectively.
 */
class ITrayMenu
{
public:
	virtual ~ITrayMenu () {}

	/** @brief Returns the list of actions to insert into tray icon menu.
	 *
	 * @return The list of actions.
	 */
	virtual QList<QAction*> GetTrayActions () const = 0;

	/** @brief Returns the list of menus to insert into tray icon menu.
	 *
	 * @return The list of menus.
	 */
	virtual QList<QMenu*> GetTrayMenus () const = 0;
};

Q_DECLARE_INTERFACE (ITrayMenu, "org.Deviant.LeechCraft.ITrayMenu/1.0");

#endif


