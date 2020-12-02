/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QShortcut>
#include <QList>
#include "shortcutsconfig.h"

class QKeySequence;
class QWidget;
class QObject;

namespace LC::Util
{
	/** @brief Makes \em func invokable with shortcuts in \em seq.
	 *
	 * This function creates one QShortcut object per each key sequence in
	 * \em seqs and calls \em func when one of the shortcuts is activated.
	 *
	 * If \em seqs is empty, this function does nothing.
	 *
	 * @param[in] seqs The list of key sequences to handle.
	 * @param[in] func The function to invoke when one of key sequences in
	 * \em seqs is activated.
	 * @param[in] parent The parent object for the shortcuts.
	 *
	 * @ingroup ShortcutsUtil
	 */
	UTIL_SHORTCUTS_API void CreateShortcuts (const QList<QKeySequence>& seqs,
			const std::function<void ()>& func, QWidget *parent);

	template<typename Obj>
	void CreateShortcuts (const QList<QKeySequence>& seqs,
			Obj *obj, void (Obj::*member) (), QWidget *parent)
	{
		for (const auto& sc : seqs)
			new QShortcut { sc, parent, obj, member };
	}

	/** @brief Makes \em metamethod invokable with shortcuts in \em seq.
	 *
	 * This function creates one QShortcut object per each key sequence in
	 * \em seqs and calls \em metamethod of the given \em object when one
	 * of the shortcuts is activated.
	 *
	 * If \em seqs is empty, this function does nothing.
	 *
	 * @param[in] seqs The list of key sequences to handle.
	 * @param[in] object The object whose metamethod should be invoked.
	 * @param[in] metamethod The metamethod to invoke when one of key
	 * sequences in \em seqs is activated.
	 * @param[in] parent The parent object for the shortcuts.
	 *
	 * @ingroup ShortcutsUtil
	 */
	UTIL_SHORTCUTS_API void CreateShortcuts (const QList<QKeySequence>& seqs,
			QObject *object, const char *metamethod, QWidget *parent);
}
