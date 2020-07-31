/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QByteArray>
#include <QUrl>
#include <QtPlugin>
#include <util/sll/eitherfwd.h>
#include "structures.h"

template<typename>
class QFuture;

struct EntityTestHandleResult;

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
class Q_DECL_EXPORT IDownload
{
public:
	struct Error
	{
		enum class Type
		{
			Unknown,
			NoError,
			NotFound,
			Gone,
			AccessDenied,
			AuthRequired,
			ProtocolError,
			NetworkError,
			ContentError,
			ProxyError,
			ServerError,
			LocalError,
			UserCanceled
		} Type_;

		QString Message_;

		bool operator== (const Error& other) const
		{
			return Type_ == other.Type_ && Message_ == other.Message_;
		}
	};

	struct Success {};

	using Result = LC::Util::Either<Error, Success>;

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
	 * @param[in] entity A Entity structure.
	 * @return The result of the test whether the \em entity can be
	 * handled.
	 *
	 * @sa AddJob
	 * @sa LC::Entity
	 */
	virtual EntityTestHandleResult CouldDownload (const LC::Entity& entity) const = 0;

	/** @brief Adds the job with given parameters.
	 *
	 * Adds the job to the downloader and returns the ID of the newly
	 * added job back to identify it.
	 *
	 * @param[in] entity A Entity structure.
	 * @return ID of the job for the other plugins to use.
	 *
	 * @sa LC::Entity
	 */
	virtual QFuture<Result> AddJob (LC::Entity entity) = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IDownload () {}
};

Q_DECLARE_INTERFACE (IDownload, "org.Deviant.LeechCraft.IDownload/1.0")

Q_DECLARE_METATYPE (QList<QUrl>)
