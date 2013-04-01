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

#include "widthiconprovider.h"
#include <interfaces/core/icoreproxy.h>
#include <util/utilconfig.h>

namespace LeechCraft
{
namespace Util
{
	/** @brief Provides icons from the current theme by their FDO name.
	 *
	 * This class is used to provide images from the current LeechCraft
	 * icon theme to QML.
	 *
	 * Its usage is as simple as following. First, you should add it to
	 * a QDeclarativeEngine in C++, for example:
	 * \code
	 * QDeclarativeView *view; // some QML view
	 * auto engine = view->engine ();
	 * engine->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
	 * \endcode
	 * Here proxy is the plugin proxy passed to IInfo::Init() method of
	 * your plugin.
	 *
	 * Then in QML:
	 * \code
	 * Image {
	 *     source: "image://ThemeIcons/edit-delete" + / + width
	 * }
	 * \endcode
	 * Or if there is no need in scaling:
	 * \code
	 * Image {
	 *     source: "image://ThemeIcons/edit-delete"
	 * }
	 * \endcode
	 *
	 * One could also use this with ActionButtons:
	 * \code
	 * ActionButton {
	 *     actionIconURL: "image://ThemeIcons/edit-delete"
	 * }
	 * \endcode
	 * In this case there is no need to add width parameter manually,
	 * ActionButton will take care of it.
	 */
	class UTIL_API ThemeImageProvider : public WidthIconProvider
	{
		ICoreProxy_ptr Proxy_;
	public:
		/** @brief Creates the ThemeImageProvider with the given proxy.
		 *
		 * @param[in] proxy The proxy object passed to IInfo::Init() of
		 * your plugin.
		 */
		ThemeImageProvider (ICoreProxy_ptr proxy);

		/** @brief Returns an icon from the current iconset by its ID.
		 *
		 * Reimplemented from WidthIconProvider::GetIcon().
		 *
		 * @param[in] path The icon path, like
		 * <code>QStringList ("edit-delete")</code>.
		 * @return The icon from the current iconset at the given path,
		 * or an empty icon otherwise.
		 */
		QIcon GetIcon (const QStringList& path);
	};
}
}
