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

#ifndef INTERFACES_ITAGGABLEJOBS_H
#define INTERFACES_ITAGGABLEJOBS_H
#include <QStringList>
#include <QtPlugin>

/** @brief Interface for plugins having taggable download jobs.
 *
 * If a plugin has jobs which could be tagged and wants LeechCraft to
 * know about it then it should implement this interface, but it would
 * not be used if the plugin doesn't implement IDownload and IJobHolder.
 * Tags could be used later by LeechCraft to do some searches, grouping
 * and filtering, for example.
 *
 * @sa IDownload
 * @sa IJobHolder
 */
class ITaggableJobs
{
public:
	/** @brief Sets the list with tags for a job.
	 *
	 * This function should replace the list with tags for a job which
	 * is in jobRow row in the model returned by
	 * IJobHolder::GetRepresentation().
	 *
	 * @param[in] jobRow row with the job.
	 * @param[in] tagsList List with tags.
	 */
	virtual void SetTags (int jobRow, const QStringList& tagsList) = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~ITaggableJobs () {}
};

Q_DECLARE_INTERFACE (ITaggableJobs, "org.Deviant.LeechCraft.ITaggableJobs/1.0");

#endif

