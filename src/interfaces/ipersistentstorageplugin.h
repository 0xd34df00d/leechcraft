/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

class QByteArray;

/** @brief Provides access to a storage in an IPersistentStoragePlugin.
 *
 * The storage is a key-value database. The keys can be queried via the
 * Get() method and set or updated via the Set() method.
 */
class IPersistentStorage
{
public:
	/** @brief Closes this storage handle.
	 */
	virtual ~IPersistentStorage () {}

	/** @brief Checks if the given \em key exists in the storage.
	 *
	 * @param[in] key The key to check for.
	 * @return Whether the given \em key exists in the storage.
	 *
	 * @sa Get()
	 * @sa Set()
	 */
	virtual bool HasKey (const QByteArray& key) = 0;

	/** @brief Returns the value associated with the given \em key.
	 *
	 * @note This function may return a null QVariant either if the
	 * \em key was not set, or if it was set to a null QVariant before.
	 * These two cases are effectively indistinguishable.
	 *
	 * @param[in] key The key for which the value should be returned.
	 * @return The value for this key or a null QVariant if no value
	 * is set.
	 *
	 * @sa HasKey()
	 * @sa Set()
	 */
	virtual QVariant Get (const QByteArray& key) = 0;

	/** @brief Stores the \em value under the given \em key.
	 *
	 * @param[in] key The key for which the value should be returned.
	 * @param[in] value The value to associate with the key.
	 *
	 * @sa HasKey()
	 * @sa Get()
	 */
	virtual void Set (const QByteArray& key, const QVariant& value) = 0;
};

using IPersistentStorage_ptr = std::shared_ptr<IPersistentStorage>;

/** @brief Interface for plugins providing persistent (and possibly
 * secure) storage.
 *
 * A storage is basically a key-value database. Refer to
 * IPersistentStorage documentation for more details.
 *
 * The access to the storage itself is performed via special proxy
 * handles (of type IPersistentStorage) which could be requested via
 * RequestStorage().
 *
 * @sa IPersistentStorage
 */
class IPersistentStoragePlugin
{
public:
	virtual ~IPersistentStoragePlugin () {}

	/** @brief Request an proxy handle to the storage.
	 *
	 * @return The proxy that could be used to query the storage, or a
	 * null pointer if unable to get the storage.
	 */
	virtual IPersistentStorage_ptr RequestStorage () = 0;
};

Q_DECLARE_INTERFACE (IPersistentStorage, "org.LeechCraft.IPersistentStorage/1.0")
Q_DECLARE_INTERFACE (IPersistentStoragePlugin, "org.LeechCraft.IPersistentStoragePlugin/1.0")
