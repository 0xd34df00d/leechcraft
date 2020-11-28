/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include "guiconfig.h"

class QTimer;

namespace LC
{
namespace Util
{
	/** @brief Allows to hide a widget or popup after mouse leave.
	 *
	 * This class is used to automatically hide a top-level widget like a
	 * tooltip or popup after mouse has left the widget and some time has
	 * passed.
	 *
	 * This class supports performing arbitrary operations in addition to
	 * destructing the widget, just pass the corresponding slot to call
	 * in the constructor. Despite this functionality, we will say
	 * "deletion" each time we speak of the operation without noticing
	 * that any other operation is possible on the widget.
	 *
	 * It is also possible to directly start the destruction timer using
	 * the Start() function, and to stop is with its counterpart Stop().
	 *
	 * The widget on which this mixin is constructed takes ownership of
	 * this class, so there is no need to keep it around or delete it
	 * explicitly.
	 *
	 * @ingroup GuiUtil
	 */
	class UnhoverDeleteMixin : public QObject
	{
		Q_OBJECT

		QTimer *LeaveTimer_;
		bool ContainsMouse_ = false;
	public:
		/** @brief Creates the mixin for the given parent widget.
		 *
		 * @param[in] parent The widget for which should be watched for
		 * mouse leave events.
		 * @param[in] slot The slot to call when enough time has passed
		 * since mouse leave. By default it is <code>deleteLater()</code>.
		 */
		UTIL_GUI_API explicit UnhoverDeleteMixin (QObject *parent, const char *slot = SLOT (deleteLater ()));

		/** @brief Manually starts the timer.
		 *
		 * This function can be used to start the timer after a Stop().
		 *
		 * If the widget currently contains the mouse, this function does
		 * nothing.
		 *
		 * @param[in] timeout The number of milliseconds to wait before
		 * the widget is deleted, or system-default timeout.
		 *
		 * @sa Stop()
		 */
		void UTIL_GUI_API Start (std::optional<int> timeout = {});

		/** @brief Stops the previously started timer.
		 *
		 * This function stops the started destruction timer, both if it
		 * is started as the result of mouse leave or due to Start().
		 *
		 * After a Stop() the widget will never be deleted without mouse
		 * entering the widget and then leaving it again, or calling
		 * Start().
		 *
		 * This function is useful if a user is currently interacting
		 * with a logical child of the watched widget, and though the
		 * watched widget doesn't contain mouse at the moment, it still
		 * makes sense to keep it around.
		 *
		 * @sa Start()
		 */
		void UTIL_GUI_API Stop ();
	protected:
		bool eventFilter (QObject*, QEvent*) override;
	};
}
}
