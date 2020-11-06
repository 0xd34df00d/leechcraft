/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "widthiconprovider.h"
#include <interfaces/core/icoreproxy.h>
#include "qmlconfig.h"

namespace LC::Util
{
	/** @brief Provides icons from the current theme by their FDO name.
	 *
	 * This class is used to provide images from the current LeechCraft
	 * icon theme to QML.
	 *
	 * Its usage is as simple as following. First, you should add it to
	 * a QDeclarativeEngine in C++, for example:
	 * \code
		QDeclarativeView *view; // some QML view
		auto engine = view->engine ();
		engine->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
	   \endcode
	 * Here proxy is the plugin proxy passed to IInfo::Init() method of
	 * your plugin.
	 *
	 * Then in QML:
	 * \code
		Image {
			source: "image://ThemeIcons/edit-delete" + / + width
		}
	   \endcode
	 * Or if there is no need in scaling:
	 * \code
		Image {
			source: "image://ThemeIcons/edit-delete"
		}
	  \endcode
	 *
	 * One could also use this with ActionButtons:
	 * \code
		ActionButton {
			actionIconURL: "image://ThemeIcons/edit-delete"
		}
	   \endcode
	 * In this case there is no need to add width parameter manually,
	 * ActionButton will take care of it.
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API ThemeImageProvider : public WidthIconProvider
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
		 * <code>QStringList { "edit-delete" }</code>.
		 * @return The icon from the current iconset at the given path,
		 * or an empty icon otherwise.
		 */
		QIcon GetIcon (const QStringList& path) override;
	};
}
