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

class QUrl;
class QModelIndex;

namespace Media
{
	/** @brief Interface for radios supporting streams adding or removal.
	 *
	 * This interface is to be implemented by an IRadioStation that
	 * supports adding or removal streams by URL. For example, a pseudo
	 * radiostation keeping bookmarks would implement this.
	 *
	 * @sa IRadioStation
	 */
	class Q_DECL_EXPORT IModifiableRadioStation
	{
	public:
		virtual ~IModifiableRadioStation () {}

		/** @brief Adds a new item.
		 *
		 * Adds an item with the given \em url under the given \em name.
		 *
		 * @param[in] url The URL of the stream to add.
		 * @param[in] name The name of the radio station (may be empty).
		 */
		virtual void AddItem (const QUrl& url, const QString& name) = 0;

		/** @brief Removes the previously added item.
		 *
		 * This function removes an item identified by the \em index. The
		 * \em index belongs to the model returned by the
		 * IRadioStationProvider::GetRadioListItems() method of the
		 * parent IRadioStationProvider that returned this radio station.
		 *
		 * @param[in] index The index of the stream to remove.
		 */
		virtual void RemoveItem (const QModelIndex& index) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IModifiableRadioStation, "org.LeechCraft.Media.IModifiableRadioStation/1.0")
