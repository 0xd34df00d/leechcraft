/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef CHILDACTIONEVENTFILTER_H
#define CHILDACTIONEVENTFILTER_H
#include <QObject>

namespace LC
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

