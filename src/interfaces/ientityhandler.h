/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_IENTITYHANDLER_H
#define INTERFACES_IENTITYHANDLER_H
#include <QByteArray>
#include <QtPlugin>
#include "structures.h"

struct EntityTestHandleResult;

/** @brief Interface for plugins able to handle entities.
 *
 * This is similar to IDownload, but it doesn't require to implement all
 * functions of IDownload, and IDownloaders and IEntityHandlers are
 * shown in different groups when presented to user, for example, in
 * entity handle dialog.
 *
 * @sa IDownload
 */
class Q_DECL_EXPORT IEntityHandler
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
	 * @param[in] entity A Entity structure that could possibly be
	 * handled by this plugin.
	 * @return The result of testing whether the \em entity can be
	 * handled.
	 *
	 * @sa Handle
	 * @sa LC::Entity
	 */
	virtual EntityTestHandleResult CouldHandle (const LC::Entity& entity) const = 0;

	/** @brief Notifies the plugin that it should handle the given entity.
	 *
	 * This function is called to make IEntityHandle know that it should
	 * handle the given entity. The entity is guaranteed to be checked
	 * previously against CouldHandle().
	 *
	 * @param[in] entity A Entity structure to be handled by
	 * this plugin.
	 *
	 * @sa LC::Entity
	 */
	virtual void Handle (LC::Entity entity) = 0;

	virtual ~IEntityHandler () {}
};

Q_DECLARE_INTERFACE (IEntityHandler, "org.Deviant.LeechCraft.IEntityHandler/1.0")

#endif

