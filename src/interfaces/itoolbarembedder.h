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

#ifndef INTERFACES_ITOOLBAREMBEDDER_H
#define INTERFACES_ITOOLBAREMBEDDER_H
#include <QList>
#include <QAction>

/** @brief Interface to embed actions into the main toolbar.
 *
 * Plugins that want to embed some actions into the main LeechCraft
 * window's toolbar. Actions are appended to the end of the toolbar, so
 * the order of them depends upon plugin load order.
 */
class IToolBarEmbedder
{
public:
	/** @brief Returns the list of actions.
	 *
	 * Returns the list of pointers to QActions that should be embedded
	 * into the toolbar.
	 *
	 * @return The list of actions.
	 */
	virtual QList<QAction*> GetActions () const = 0; 

	virtual ~IToolBarEmbedder () {}
};

Q_DECLARE_INTERFACE (IToolBarEmbedder, "org.Deviant.LeechCraft.IToolBarEmbedder/1.0");

#endif

