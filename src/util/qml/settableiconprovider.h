/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "widthiconprovider.h"
#include <QHash>
#include <QIcon>

namespace LC::Util
{
	/** @brief QML image provider with settable icons for each path.
	 *
	 * This class implements a QML image provider that provides preset
	 * icons for given paths. The icons are set via SetIcon() and
	 * ClearIcon().
	 *
	 * @sa WidthIconProvider
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API SettableIconProvider : public WidthIconProvider
	{
		QHash<QStringList, QIcon> Icons_;
	public:
		/** @brief Sets the \em icon for the given \em path.
		 *
		 * If there is already an icon for this \em path, it is replaced
		 * by the new \em icon.
		 *
		 * The icon set with this function is available for this \em path
		 * via the GetIcon() method.
		 *
		 * @param[in] path The path associated with the \em icon.
		 * @param[in] icon The icon to associate with the \em path.
		 *
		 * @sa ClearIcon()
		 * @sa GetIcon()
		 */
		void SetIcon (const QStringList& path, const QIcon& icon);

		/** @brief Clears the icon associated with the given \em path.
		 *
		 * @param[in] path The path to clear.
		 *
		 * @sa SetIcon()
		 * @sa GetIcon()
		 */
		void ClearIcon (const QStringList& path);

		/** @brief Returns the icon for the \em path previously set with
		 * SetIcon().
		 *
		 * @param[in] path The path for which to return an the icon.
		 * @return The icon for the \em path, or a null icon of no icon
		 * is associated with it.
		 *
		 * @sa SetIcon()
		 * @sa ClearIcon()
		 */
		QIcon GetIcon (const QStringList& path) override;
	};
}
