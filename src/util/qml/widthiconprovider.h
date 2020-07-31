/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QQuickImageProvider>
#include "qmlconfig.h"

class QIcon;

namespace LC
{
namespace Util
{
	/** @brief Provides scalable icons to QML.
	 *
	 * This class is used to provide pixmaps generated from QIcons to
	 * QML. It supports generating different pixmaps for different source
	 * sizes. For that, the last component of the URL in the
	 * <code>source</code> property of a QML <code>Image</code> should
	 * correspond to the current width, like this:
	 * \code
	 * Image {
	 *     source: someIconString + '/' + width
	 *     // ...
	 * }
	 * \endcode
	 *
	 * The subclasses should implement the GetIcon() pure virtual
	 * function, which should return a QIcon for a given path, where path
	 * is a QStringList obtained by breaking the URL path at '/' and
	 * \em leaving the width component.
	 *
	 * Please also see the documentation for <code>QIcon::pixmap()</code>
	 * regarding upscaling.
	 *
	 * @sa ThemeImageProvider
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API WidthIconProvider : public QQuickImageProvider
	{
	public:
		WidthIconProvider ();

		/** @brief Reimplemented from QDeclarativeImageProvider::requestPixmap().
		 *
		 * @param[in] id The image ID.
		 * @param[in] size If non-null, will be set to the real size of
		 * the generated image.
		 * @param[in] requestedSize The requested image size.
		 */
		QPixmap requestPixmap (const QString& id, QSize *size, const QSize& requestedSize);

		/** @brief Implement this method to return a proper QIcon for path.
		 *
		 * See this class documentation for more information.
		 *
		 * @param[in] path The icon path, a list obtained by breaking the
		 * URL request path at '/'.
		 * @return QIcon for the path, or an empty QIcon.
		 */
		virtual QIcon GetIcon (const QStringList& path) = 0;
	};
}
}
