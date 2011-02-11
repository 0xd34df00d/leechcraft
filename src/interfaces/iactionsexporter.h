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

#ifndef INTERFACES_IACTIONSEXPORTER_H
#define INTERFACES_IACTIONSEXPORTER_H
#include <QList>
#include <QMap>
#include <QtPlugin>

class QAction;

namespace LeechCraft
{
	enum ActionsEmbedPlace
	{
		AEPToolsMenu,
		AEPCommonContextMenu,
		AEPQuickLaunch,
		AEPTrayMenu
	};
}

/** @brief Interface for embedding actions and menus into various
 * places.
 */
class IActionsExporter
{
public:
	virtual ~IActionsExporter () {}

	/** @brief Returns the actions to embed.
	 *
	 * Returns the list of actions that will be inserted into the Tools
	 * menu.
	 *
	 * @return The list of actions.
	 */
	virtual QList<QAction*> GetActions (LeechCraft::ActionsEmbedPlace) const = 0;
	
	/** @brief Returns the actions to embed into the menu.
	 * 
	 * For each string key found in the returned map, the corresponding
	 * list of QActions would be added to the submenu under that name in
	 * the main menu. That allows several different plugins to insert
	 * actions into one menu easily.
	 * 
	 * @return The map of menu name -> list of its actions.
	 */
	virtual QMap<QString, QList<QAction*> > GetMenuActions () const
	{
		return QMap<QString, QList<QAction*> > ();
	}
};

Q_DECLARE_INTERFACE (IActionsExporter, "org.Deviant.LeechCraft.IActionsExporter/1.0");

#endif
