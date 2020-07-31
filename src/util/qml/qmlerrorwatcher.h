/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "qmlconfig.h"

class QQuickWidget;

namespace LC
{
namespace Util
{
	/** @brief Utility class for logging errors from a QML view.
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API QmlErrorWatcher : public QObject
	{
	public:
		/** @brief Constructs the watcher for the given \em view.
		 *
		 * The \em view becomes the parent of the watcher, so there no
		 * need to delete the watcher explicitly later on.
		 *
		 * @param[in] view The declarative view to watch for errors.
		 */
		QmlErrorWatcher (QQuickWidget *view);
	};
}
}
