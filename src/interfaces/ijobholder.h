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

#ifndef INTERFACES_IJOBHOLDER_H
#define INTERFACES_IJOBHOLDER_H
#include <QtPlugin>

class QModelIndex;
class QAbstractItemModel;
class QWidget;

/** @brief Interface for plugins providing data for the Downloaders tab.
 *
 * If a plugin wants to show any data in the Downloaders tab, it should
 * implement this interface.
 *
 * Item model is returned by GetRepresentation(), and various roles are
 * used to retrieve controls and information pane of the plugin.
 * Returned model should have four columns, each for name, state and
 * status. Controls and additional information pane are only visible
 * when a job handled by the plugin is selected.
 *
 * @sa IDownloader
 * @sa CustomDataRoles
 */
class IJobHolder
{
public:
	/** @brief Returns the item representation model.
	 *
	 * The returned model should have four columns, each for name,
	 * state, progress and speed. Inside of LeechCraft it would be
	 * merged with other models from other plugins.
	 *
	 * This model is also used to retrieve controls and additional info
	 * for a given index.
	 *
	 * Returned widget would be placed above the view with the jobs, so
	 * usually it has controls of the job, but in fact it can have
	 * anything you want. It is only visible when a job from your plugin
	 * is selected. If a job from other plugin is selected, then other
	 * plugin's controls would be placed, and if no jobs are selected at
	 * all, then no controls would be placed.
	 *
	 * Additional information a retrieved using the RoleAdditionalInfo.
	 * Returned widget would be placed below the view with the jobs, so
	 * usually it has additional information about the job, but in fact
	 * it can have anything you want. It is only visible when a job from
	 * your plugin is selected. If a job from other plugin is selected,
	 * then other plugin's controls would be placed, and if no jobs are
	 * selected at all, then no controls would be placed.
	 *
	 * @return Representation model.
	 *
	 * @sa CustomDataRoles
	 */
	virtual QAbstractItemModel* GetRepresentation () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IJobHolder () {}
};

Q_DECLARE_METATYPE (QAbstractItemModel*);
Q_DECLARE_METATYPE (QList<QModelIndex>);
Q_DECLARE_INTERFACE (IJobHolder, "org.Deviant.LeechCraft.IJobHolder/1.0");

#endif

