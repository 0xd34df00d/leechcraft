/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtglobalshortcut.h"
#include "qxtglobalshortcut_p.h"
#include <QAbstractEventDispatcher>
#include <QtDebug>

bool QxtGlobalShortcutPrivate::error = false;
#ifndef Q_OS_MAC
int QxtGlobalShortcutPrivate::ref = 0;
QList<QxtGlobalShortcutPrivate*> QxtGlobalShortcutPrivate::allPrivates;
#endif // Q_OS_MAC
QHash<QPair<quint32, quint32>, QxtGlobalShortcut*> QxtGlobalShortcutPrivate::shortcuts;

QxtGlobalShortcutPrivate::QxtGlobalShortcutPrivate() : enabled(true), key(Qt::Key(0)), mods(Qt::NoModifier)
{
#ifndef Q_OS_MAC
	if (allPrivates.isEmpty ())
		QAbstractEventDispatcher::instance ()->installNativeEventFilter (this);
	allPrivates << this;
#endif // Q_OS_MAC
}

QxtGlobalShortcutPrivate::~QxtGlobalShortcutPrivate()
{
#ifndef Q_OS_MAC
	const auto isActiveFilter = allPrivates.value (0) == this;
	allPrivates.removeOne (this);

	if (isActiveFilter && !allPrivates.isEmpty ())
		QAbstractEventDispatcher::instance ()->installNativeEventFilter (allPrivates.first ());
#endif // Q_OS_MAC
}

bool QxtGlobalShortcutPrivate::setShortcut(const QKeySequence& shortcut)
{
	key = shortcut[0].key();
	mods = shortcut[0].keyboardModifiers ();
	const quint32 nativeKey = nativeKeycode(key);
	const quint32 nativeMods = nativeModifiers(mods);
	const bool res = registerShortcut(nativeKey, nativeMods);
	shortcuts.insert(qMakePair(nativeKey, nativeMods), &qxt_p());
	if (!res)
		qWarning() << "QxtGlobalShortcut failed to register:" << QKeySequence(QKeyCombination(mods, key)).toString();
	return res;
}

bool QxtGlobalShortcutPrivate::unsetShortcut()
{
	const quint32 nativeKey = nativeKeycode(key);
	const quint32 nativeMods = nativeModifiers(mods);
	const bool res = unregisterShortcut(nativeKey, nativeMods);
	shortcuts.remove(qMakePair(nativeKey, nativeMods));
	if (!res)
		qWarning() << "QxtGlobalShortcut failed to unregister:" << QKeySequence(QKeyCombination(mods, key)).toString();
	key = Qt::Key(0);
	mods = {};
	return res;
}

void QxtGlobalShortcutPrivate::activateShortcut(quint32 nativeKey, quint32 nativeMods)
{
	QxtGlobalShortcut* shortcut = shortcuts.value(qMakePair(nativeKey, nativeMods));
	if (shortcut && shortcut->isEnabled())
		emit shortcut->activated();
}

/*!
	\class QxtGlobalShortcut
	\inmodule QxtGui
	\brief The QxtGlobalShortcut class provides a global shortcut aka "hotkey".

	A global shortcut triggers even if the application is not active. This
	makes it easy to implement applications that react to certain shortcuts
	still if some other application is active or if the application is for
	example minimized to the system tray.

	Example usage:
	\code
	QxtGlobalShortcut* shortcut = new QxtGlobalShortcut(window);
	connect(shortcut, SIGNAL(activated()), window, SLOT(toggleVisibility()));
	shortcut->setShortcut(QKeySequence("Ctrl+Shift+F12"));
	\endcode

	\bold {Note:} Since Qxt 0.6 QxtGlobalShortcut no more requires QxtApplication.
 */

/*!
	\fn QxtGlobalShortcut::activated()

	This signal is emitted when the user types the shortcut's key sequence.

	\sa shortcut
 */

/*!
	Constructs a new QxtGlobalShortcut with \a parent.
 */
QxtGlobalShortcut::QxtGlobalShortcut(QObject* parent)
		: QObject(parent)
{
	QXT_INIT_PRIVATE(QxtGlobalShortcut);
}

/*!
	Constructs a new QxtGlobalShortcut with \a shortcut and \a parent.
 */
QxtGlobalShortcut::QxtGlobalShortcut(const QKeySequence& shortcut, QObject* parent)
		: QObject(parent)
{
	QXT_INIT_PRIVATE(QxtGlobalShortcut);
	setShortcut(shortcut);
}

/*!
	Destructs the QxtGlobalShortcut.
 */
QxtGlobalShortcut::~QxtGlobalShortcut()
{
	if (qxt_d().key != 0)
		qxt_d().unsetShortcut();
}

/*!
	\property QxtGlobalShortcut::shortcut
	\brief the shortcut key sequence

	\bold {Note:} Notice that corresponding key press and release events are not
	delivered for registered global shortcuts even if they are disabled.
	Also, comma separated key sequences are not supported.
	Only the first part is used:

	\code
	qxtShortcut->setShortcut(QKeySequence("Ctrl+Alt+A,Ctrl+Alt+B"));
	Q_ASSERT(qxtShortcut->shortcut() == QKeySequence("Ctrl+Alt+A"));
	\endcode
 */
QKeySequence QxtGlobalShortcut::shortcut() const
{
	return QKeySequence(qxt_d().key | qxt_d().mods);
}

bool QxtGlobalShortcut::setShortcut(const QKeySequence& shortcut)
{
	if (qxt_d().key != 0)
		qxt_d().unsetShortcut();
	return qxt_d().setShortcut(shortcut);
}

/*!
	\property QxtGlobalShortcut::enabled
	\brief whether the shortcut is enabled

	A disabled shortcut does not get activated.

	The default value is \c true.

	\sa setDisabled()
 */
bool QxtGlobalShortcut::isEnabled() const
{
	return qxt_d().enabled;
}

void QxtGlobalShortcut::setEnabled(bool enabled)
{
	qxt_d().enabled = enabled;
}

/*!
	Sets the shortcut \a disabled.

	\sa enabled
 */
void QxtGlobalShortcut::setDisabled(bool disabled)
{
	qxt_d().enabled = !disabled;
}
