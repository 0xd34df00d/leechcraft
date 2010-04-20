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

#ifndef INTERFACES_IENTITYHANDLER_H
#define INTERFACES_IENTITYHANDLER_H
#include <QByteArray>
#include <QtPlugin>
#include "structures.h"

/** @brief Interface for plugins able to handle entities.
 *
 * This is similar to IDownload, but it doesn't require to implement all
 * functions of IDownload, and IDownloaders and IEntityHandlers are
 * shown in different groups when presented to user, for example, in
 * entity handle dialog.
 *
 * @sa IDownload
 */
class IEntityHandler
{
public:
	/** @brief Returns whether plugin can handle given entity.
	 *
	 * This function is used to query every loaded plugin providing the
	 * IDownload interface whether it could handle the entity entered by
	 * user or generated automatically with given task parameters.
	 * Entity could be anything from file name to URL to all kinds of
	 * hashes like Magnet links.
	 *
	 * @param[in] entity A DownloadEntity structure that could be possibly
	 * handled by this plugin.
	 * @return Whether this plugin can handle this particular entity.
	 *
	 * @sa Handle
	 * @sa LeechCraft::DownloadEntity
	 */
	virtual bool CouldHandle (const LeechCraft::DownloadEntity& entity) const = 0;

	/** @brief Notifies the plugin that it should handle the given entity.
	 *
	 * This function is called to make IEntityHandle know that it should
	 * handle the given entity. The entity is guaranteed to be checked
	 * previously against CouldHandle().
	 *
	 * @param[in] entity A DownloadEntity structure to be handled by
	 * this plugin.
	 *
	 * @sa LeechCraft::DownloadEntity
	 */
	virtual void Handle (LeechCraft::DownloadEntity entity) = 0;

	virtual ~IEntityHandler () {}
};

Q_DECLARE_INTERFACE (IEntityHandler, "org.Deviant.LeechCraft.IEntityHandler/1.0");

#endif

