/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPoint>
#include "guiconfig.h"

class QRect;
class QScreen;
class QSize;

namespace LC::Util
{
	/** Additional fitting options used by FitRect() and FitRectScreen().
	 *
	 * @ingroup GuiUtil
	 */
	enum FitFlag
	{
		/** Default fitting options.
		 */
		NoFlags,

		/** The fitted rectangle should never overlap with the original
		 * top left position.
		 *
		 * This option may be useful if a menu or tooltip is fitted,
		 * since it's generally undesirable to cover the corresponding
		 * UI element with the menu or tooltip.
		 */
		NoOverlap = 0x01
	};

	Q_DECLARE_FLAGS (FitFlags, FitFlag);

	/** @brief Tries to fit a rectangle (like a dialog or popup) into screen.
	 *
	 * This function tries to move the rectangle with top left point at
	 * pos and with given size so that it fits completely into the
	 * available geometry of the screen that contains the point pos. It
	 * leaves the rectangle size intact, instead returning the new top
	 * left position.
	 *
	 * Calling this function is equivalent to calling FitRect() with
	 * the geometry parameter set to
	 * <code>QDesktopWidget::availableGeometry(pos)</code>. See the
	 * documentation for FitRect() for more details.
	 *
	 * @param[in] pos The original top left position of the rect to fit.
	 * @param[in] size The size of the rectangle to fit.
	 * @param[in] flags Additional fitting parameters.
	 * @param[in] shiftAdd Additional components to be added if the
	 * rectangle is actually moved in the corresponding directions.
	 * @return The new top left position of the rectangle.
	 *
	 * @sa FitRect()
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API QPoint FitRectScreen (QPoint pos, const QSize& size,
			FitFlags flags = NoFlags, const QPoint& shiftAdd = QPoint (0, 0));

	/** @brief Tries to fit a rectangle (like a dialog or popup) into geometry.
	 *
	 * This function tries to move the rectangle with top left point at
	 * pos and with given size so that it fits completely into the
	 * rectangle given by the geometry parameter. It leaves the
	 * size intact, instead returning the new top left position.
	 *
	 * If the rectangle is actually moved by this function, the shiftAdd
	 * parameter is used to customize how it is moved: the
	 * <code>shiftAdd.x()</code> component is added to the result iff
	 * <code>pos.x()</code> is changed, and <code>shiftAdd.y()</code> is
	 * added to the result iff <code>pos.y()</code> is changed.
	 *
	 * @param[in] pos The original top left position of the rect to fit.
	 * @param[in] size The size of the rectangle to fit.
	 * @param[in] geometry The rectangle \em into which the source
	 * rectangle should be fitted.
	 * @param[in] flags Additional fitting parameters.
	 * @param[in] shiftAdd Additional components to be added if the
	 * rectangle is actually moved in the corresponding directions.
	 * @return The new top left position of the rectangle.
	 *
	 * @sa FitRectScreen()
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API QPoint FitRect (QPoint pos, const QSize& size, const QRect& geometry,
			FitFlags flags = NoFlags, const QPoint& shiftAdd = QPoint (0, 0));

	UTIL_GUI_API QScreen* GetScreenWithFallback (const QPoint& p);
	UTIL_GUI_API QRect AvailableGeometry (const QPoint& p);
	UTIL_GUI_API QRect ScreenGeometry (const QPoint& p);

}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Util::FitFlags)
