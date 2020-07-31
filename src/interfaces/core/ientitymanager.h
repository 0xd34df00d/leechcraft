/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFuture>
#include <util/sll/either.h>
#include <interfaces/idownload.h>

namespace LC
{
	struct Entity;
}

/** @brief Proxy to core entity manager.
 *
 * Core entity manager is that very thing that routes entities between
 * different plugins and chooses which plugins will handle what entity.
 *
 * This class can be used instead of the more or less deprecated
 * IInfo::gotEntity() signal.
 *
 * @sa Entity, IInfo
 */
class Q_DECL_EXPORT IEntityManager
{
public:
	/** @brief The result of delegating this entity to another plugin.
	 */
	struct DelegationResult
	{
		/** @brief The plugin instance object that handles this entity.
		 *
		 * If no object handles the entity, this is a nullptr.
		 */
		QObject *Handler_ = nullptr;

		/** The future with the download result.
		 */
		QFuture<IDownload::Result> DownloadResult_;

		explicit operator bool () const
		{
			return Handler_;
		}
	};

	virtual ~IEntityManager () {}

	/** @brief Delegates the given entity and returns the delegation result.
	 *
	 * Queries all plugins if they can handle the given entity, and
	 * chooses one of them. If the desired object is set, this method
	 * first tries to handle the entity with it. Returns a structure
	 * describing the delegation result.
	 *
	 * This function shall only be called from the UI thread.
	 *
	 * @param[in] entity The entity to handle.
	 * @param[in] desired The object to try first.
	 *
	 * @return A structure descripting the result of the delegation request.
	 *
	 * @sa DelegationResult
	 */
	virtual DelegationResult DelegateEntity (LC::Entity entity, QObject *desired = nullptr) = 0;

	/** @brief Handles the given entity.
	 *
	 * Queries all plugins if they can handle the given entity, and
	 * chooses one of them (or all of them, according to entity flags and
	 * plugins' behavior). If the desired object is set, this method
	 * first tries to handle the entity with it.
	 *
	 * It is safe to call this method from non-UI threads.
	 *
	 * @param[in] entity The entity to handle.
	 * @param[in] desired The object to try first.
	 *
	 * @return If the entity has been handled successfully.
	 */
	virtual bool HandleEntity (LC::Entity entity, QObject *desired = nullptr) = 0;

	/** @brief Queries whether the given entity can be handled at all.
	 *
	 * @param[in] entity The entity to test.
	 *
	 * @return Whether there is at least one plugin to handle this entity.
	 */
	virtual bool CouldHandle (const LC::Entity& entity) = 0;

	/** @brief Queries what plugins can handle the given entity.
	 *
	 * @param[in] entity The entity to test.
	 *
	 * @return The list of plugin instances that can handle the given entity.
	 */
	virtual QList<QObject*> GetPossibleHandlers (const LC::Entity& entity) = 0;
};

Q_DECLARE_INTERFACE (IEntityManager, "org.Deviant.LeechCraft.IEntityManager/1.0")
