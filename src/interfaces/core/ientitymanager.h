/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QObject>

namespace LeechCraft
{
	struct Entity;
}

/** @brief Proxy to core entity manager.
 *
 * Core entity manager is that very thing that routes entities between
 * different plugins and chooses which plugins will handle what entity.
 *
 * This class can be used instead more or less deprecated gotEntity()
 * and delegateEntity() signals of IInfo.
 *
 * @sa Entity, IInfo
 */
class Q_DECL_EXPORT IEntityManager
{
public:
	/** The result of delegating this entity to another plugin.
	 */
	struct DelegationResult
	{
		/** The plugin instance object that handles this entity.
		 *
		 * If there is no such object, it's a nullptr.
		 */
		QObject *Handler_;

		/** The internal ID of the handling event in "namespace" of
		 * the Handler_ plugin.
		 */
		int ID_;
	};

	virtual ~IEntityManager () {}

	/** @brief Delegates the given entity and returns the delegation result.
	 *
	 * Queries all plugins if they can handle the given entity, and
	 * chooses one of them. If the desired object is set, this method
	 * first tries to handle the entity with it. Returns a structure
	 * describing the delegation result.
	 *
	 * @param[in] entity The entity to handle.
	 * @param[in] desired The object to try first.
	 *
	 * @sa DelegationResult
	 */
	virtual DelegationResult DelegateEntity (LeechCraft::Entity entity, QObject *desired = 0) = 0;

	/** @brief Handles the given entity.
	 *
	 * Queries all plugins if they can handle the given entity, and
	 * chooses one of them (or all of them, according to entity flags and
	 * plugins' behavior). If the desired object is set, this method
	 * first tries to handle the entity with it.
	 *
	 * @param[in] entity The entity to handle.
	 * @param[in] desired The object to try first.
	 *
	 * @return If the entity has been handled successfully.
	 */
	virtual bool HandleEntity (LeechCraft::Entity entity, QObject *desired = 0) = 0;

	/** @brief Queries whether the given entity can be handled at all.
	 *
	 * @param[in] entity The entity to test.
	 *
	 * @return Whether there is at least one plugin to handle this entity.
	 */
	virtual bool CouldHandle (const LeechCraft::Entity& entity) = 0;

	/** @brief Queries what plugins can handle the given entity.
	 *
	 * @param[in] entity The entity to test.
	 *
	 * @return The list of plugin instances that can handle the given entity.
	 */
	virtual QList<QObject*> GetPossibleHandlers (const LeechCraft::Entity& entity) = 0;
};

Q_DECLARE_INTERFACE (IEntityManager, "org.Deviant.LeechCraft.IEntityManager/1.0");
