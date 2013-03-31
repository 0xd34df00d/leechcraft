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

#include <QObject>
#include <util/utilconfig.h>

class QTimer;

namespace LeechCraft
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
	 */
	class UnhoverDeleteMixin : public QObject
	{
		QTimer *LeaveTimer_;
		bool ContainsMouse_;
	public:
		/** @brief Creates the mixin for the given parent widget.
		 *
		 * @param[in] parent The widget for which should be watched for
		 * mouse leave events.
		 * @param[in] slot The slot to call when enough time has passed
		 * since mouse leave. By default it is <code>deleteLater()</code>.
		 */
		UTIL_API UnhoverDeleteMixin (QWidget *parent, const char *slot = SLOT (deleteLater ()));

		/** @brief Manually starts the timer.
		 *
		 * This function can be used to start the timer after a Stop().
		 *
		 * If the widget currently contains the mouse, this function does
		 * nothing.
		 *
		 * @param[in] timeout The number of milliseconds to wait before
		 * the widget is deleted..
		 *
		 * @sa Stop()
		 */
		void UTIL_API Start (int timeout = 1200);

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
		void UTIL_API Stop ();
	protected:
		bool eventFilter (QObject*, QEvent*);
	};
}
}
