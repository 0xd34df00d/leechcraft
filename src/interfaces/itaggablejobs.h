/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
class Q_DECL_EXPORT ITaggableJobs
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

Q_DECLARE_INTERFACE (ITaggableJobs, "org.Deviant.LeechCraft.ITaggableJobs/1.0")

#endif

