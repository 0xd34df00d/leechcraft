/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef CHILDACTIONEVENTFILTER_H
#define CHILDACTIONEVENTFILTER_H
#include <QObject>

namespace LeechCraft
{
	/** Event filter to intercept child creation/polish events to
	 * properly set icons on QActions. This event filter is supposed to
	 * be installed on MainWindow.
	 */
	class ChildActionEventFilter : public QObject
	{
		Q_OBJECT
	public:
		/** Creates the event filter.
		 */
		ChildActionEventFilter (QObject* = 0);

		/** Destroys the event filter.
		 */
		virtual ~ChildActionEventFilter ();
	protected:
		/** If event type type is QEvent::ChildAdded or
		 * QEvent::ChildPolished and the child is a QAction*, it sets
		 * a proper icon for that QAction according to its ActinIcon
		 * property value.
		 *
		 * @param[in] event The event.
		 * @param[in] object The object watched (usually the MainWindow).
		 */
		bool eventFilter (QObject *object, QEvent *event);
	};
};

#endif

