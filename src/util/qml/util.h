/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "qmlconfig.h"

class QQuickWidget;

namespace LC::Util
{
	UTIL_QML_API void EnableTransparency (QQuickWidget& widget);

	UTIL_QML_API void SetupFullscreenView (QQuickWidget& widget);

	/** @brief Logs errors from a QML view.
	 *
	 * @param[in] view The declarative view to watch for errors.
	 *
	 * @ingroup QmlUtil
	 */
	UTIL_QML_API void WatchQmlErrors (QQuickWidget& view);
}
