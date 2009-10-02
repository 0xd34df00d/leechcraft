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

#ifndef INTERFACES_IMENUEMBEDDER_H
#define INTERFACES_IMENUEMBEDDER_H
#include <QList>
#include <QAction>

/** @brief Interface to embed actions into the Tools menu.
 *
 * Plugins that want to embed their menus into the main LeechCraft's menu
 * should implement this interface. Plugin's menus are inserted into the
 * Tools menu.
 */
class IMenuEmbedder
{
public:
	/** @brief Returns the menus to embed.
	 *
	 * Returns the list of menus that will be inserted into the Tools
	 * menu.
	 *
	 * @return The list of menus.
	 */
	virtual QList<QMenu*> GetToolMenus () const = 0; 

	/** @brief Returns the actions to embed.
	 *
	 * Returns the list of actions that will be inserted into the Tools
	 * menu.
	 *
	 * @return The list of actions.
	 */
	virtual QList<QAction*> GetToolActions () const = 0; 

	virtual ~IMenuEmbedder () {}
};

Q_DECLARE_INTERFACE (IMenuEmbedder, "org.Deviant.LeechCraft.IMenuEmbedder/1.0");

#endif

