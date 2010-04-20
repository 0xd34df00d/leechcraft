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

#ifndef INTERFACES_IDOWNLOAD_H
#define INTERFACES_IDOWNLOAD_H
#include <QByteArray>
#include <QtPlugin>
#include "structures.h"

/** @brief Common interface for all the downloaders.
 *
 * Plugins which provide downloading capabilities and want to be visible
 * by LeechCraft and other plugins as download providers should
 * implement this interface.
 *
 * Plugins implementing this interface are expected to have following
 * signals:
 * - jobFinished (int id)
 *   Indicates that a job with a given id has finished.
 * - jobFinished (int id)
 *   Indicates that a job with a given id has been removed.
 * - handleJobError (int id, IDownload::Error error)
 *   Indicates that an error occured while downloading a job with the
 *   given id.
 *
 * In order to obtain IDs for the tasks plugins are expected to use
 * ICoreProxy::GetID() in order to avoid name clashes.
 *
 * @sa IJobHolder, IEntityHandler
 */
class IDownload
{
public:
	enum Error
	{
		EUnknown
		, ENoError
		, ENotFound
		, EAccessDenied
		, ELocalError
	};
	typedef unsigned long int JobID_t;

	/** @brief Returns download speed.
	 *
	 * Returns summed up download speed of the plugin. The value is
	 * primarily used in the interface as there are no ways of
	 * controlling of bandwidth's usage of a particular plugin.
	 *
	 * @return Download speed in bytes.
	 *
	 * @sa GetUploadSpeed
	 */
	virtual qint64 GetDownloadSpeed () const = 0;
	/** @brief Returns upload speed.
	 *
	 * Returns summed up upload speed of the plugin. The value is
	 * primarily used in the interface as there are no ways of
	 * controlling of bandwidth's usage of a particular plugin.
	 *
	 * @return Upload speed in bytes.
	 *
	 * @sa GetDownloadSpeed
	 */
	virtual qint64 GetUploadSpeed () const = 0;

	/** @brief Starts all tasks.
	 *
	 * This is called by LeechCraft when it wants all plugins to start
	 * all of its tasks.
	 */
	virtual void StartAll () = 0;
	/** @brief Stops all tasks.
	 *
	 * This is called by LeechCraft when it wants all plugins to stop
	 * all of its tasks.
	 */
	virtual void StopAll () = 0;

	/** @brief Returns whether plugin can handle given entity.
	 *
	 * This function is used to query every loaded plugin providing the
	 * IDownload interface whether it could handle the entity entered by
	 * user or generated automatically with given task parameters.
	 * Entity could be anything from file name to URL to all kinds of
	 * hashes like Magnet links.
	 *
	 * @param[in] entity A DownloadEntity structure.
	 *
	 * @sa AddJob
	 * @sa LeechCraft::DownloadEntity
	 */
	virtual bool CouldDownload (const LeechCraft::DownloadEntity& entity) const = 0;

	/** @brief Adds the job with given parameters.
	 *
	 * Adds the job to the downloader and returns the ID of the newly
	 * added job back to identify it.
	 *
	 * @param[in] entity A DownloadEntity structure.
	 * @return ID of the job for the other plugins to use.
	 *
	 * @sa LeechCraft::DownloadEntity
	 */
	virtual int AddJob (LeechCraft::DownloadEntity entity) = 0;

	/** @brief Kills the task with the given id.
	 *
	 * Kills the task with the id previously returned from AddJob. If
	 * there is no such task, the function shall leave the downloader
	 * in a good state. Ignoring will do.
	 *
	 * @param[in] id ID of the task previously added with AddJob().
	 */
	virtual void KillTask (int id) = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IDownload () {}
};

Q_DECLARE_INTERFACE (IDownload, "org.Deviant.LeechCraft.IDownload/1.0");

#endif

