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

#include <QPoint>
#include <util/utilconfig.h>

class QSize;
class QRect;
class QPixmap;
class QLabel;

namespace LeechCraft
{
namespace Util
{
	/** Additional fitting options used by FitRect() and FitRectScreen().
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
	 */
	UTIL_API QPoint FitRectScreen (QPoint pos, const QSize& size,
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
	 */
	UTIL_API QPoint FitRect (QPoint pos, const QSize& size, const QRect& geometry,
			FitFlags flags = NoFlags, const QPoint& shiftAdd = QPoint (0, 0));

	/** @brief Shows a pixmap at the given pos.
	 *
	 * This function shows a dialog with the given pixmap at the given
	 * position. If the pixmap is too big, it is scaled down. A QLabel
	 * created with window decorations is used as the dialog. The created
	 * label is returned from the function, so one could also set the
	 * window title and further customize the label.
	 *
	 * This function is useful to display full version of album art in a
	 * media player or a user avatar in an IM application.
	 *
	 * @param[in] pixmap The pixmap to show.
	 * @param[in] pos The position where the dialog should be shown.
	 * @return The created dialog.
	 */
	UTIL_API QLabel* ShowPixmapLabel (const QPixmap& pixmap, const QPoint& pos = QPoint ());
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::Util::FitFlags);
