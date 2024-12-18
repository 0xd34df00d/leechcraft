/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QPoint>
#include <QRect>
#include "guiconfig.h"

class QWidget;
class QWindow;

namespace LC::Util
{
	/** @brief Automatically moves a widget to fit a rectangle on resize.
	 *
	 * An instance of this class manages a single widget and moves it
	 * automatically after the widget has been resized, so that it stays
	 * inside the given rectangle. The autoresize mixin never tries
	 * to resize the widget itself (despite the name perhaps).
	 *
	 * This class also tries to keep a corner of the managed widget near
	 * a given point passed to the constructor. This is useful when the
	 * widget pops up in response to some mouse-initiated action so that
	 * it appears near the mouse cursor.
	 *
	 * Most commonly this function is used to keep various popup widgets
	 * on screen, but it can also be used to prefer showing some popup
	 * inside the LeechCraft window, for example.
	 *
	 * The rectangle into which the widget should be embedded is obtained
	 * via a functor returning the rectangle. The functor is invoked each
	 * time the widget is to be refit.
	 *
	 * @ingroup GuiUtil
	 */
	class AutoResizeMixin : public QObject
	{
		const QPoint OrigPoint_;

		const std::function<void (QPoint)> Mover_;
	public:
		/** @brief A function type used to get the rect to fit widget in.
		 */
		using RectGetter_f = std::function<QRect ()>;
	private:
		const RectGetter_f Rect_;
	public:
		/** @brief Constructs the resize mixin.
		 *
		 * This function constructs the resize mixin managing the given
		 * \em widget, trying to fit it inside the \em rect, preferably
		 * with a corner of the \em widget sticking near the \em point.
		 *
		 * @param[in] point The point near which the \em widget should be
		 * shown.
		 * @param[in] rect The functor returning the rectangle into which
		 * the \em widget should be fitted.
		 * @param[in] widget The widget to fit.
		 */
		UTIL_GUI_API AutoResizeMixin (const QPoint& point, RectGetter_f rect, QWidget *widget);

		/** @brief Constructs the resize mixin.
		 *
		 * This function constructs the resize mixin managing the given
		 * \em window, trying to fit it inside the \em rect, preferably
		 * with a corner of the \em window sticking near the \em point.
		 *
		 * @param[in] point The point near which the \em window should be
		 * shown.
		 * @param[in] rect The functor returning the rectangle into which
		 * the \em window should be fitted.
		 * @param[in] window The widget to fit.
		 */
		UTIL_GUI_API AutoResizeMixin (const QPoint& point, RectGetter_f rect, QWindow *window);

		/** @brief Listens for resize events and refits the widget.
		 */
		bool eventFilter (QObject*, QEvent*) override;
	private:
		void Refit (const QSize&);
	};
}
