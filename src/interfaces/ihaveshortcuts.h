/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QMultiMap>
#include <QString>
#include <QKeySequence>
#include <QIcon>
#include <QMetaType>

class QAction;

using QKeySequences_t = QList<QKeySequence>;
Q_DECLARE_METATYPE (QKeySequences_t)

namespace LC
{
	/** @brief Describes an action exposed in shortcut manager.
	 *
	 * This structure contains information about the action that could be
	 * handled by the shortcut manager, like action icon, default key
	 * sequences and human-readable description text.
	 */
	struct ActionInfo
	{
		/// User-visible name of the action.
		QString UserVisibleText_;
		/// List of key sequences for this action.
		QKeySequences_t Seqs_;
		/// Icon of the action.
		QIcon Icon_;

		/** @brief Default-constructs an action info.
		 */
		ActionInfo ()
		{
		}

		/** @brief Constructs an action info.
		 *
		 * Constructs an info object for the given user-visible text
		 * \em uvt, default key sequence \em seq and action icon
		 * \em icon.
		 */
		ActionInfo (const QString& uvt,
				const QKeySequence& seq,
				const QIcon& icon)
		: UserVisibleText_ (uvt)
		, Icon_ (icon)
		{
			Seqs_ << seq;
		}

		/** @brief Constructs an action info.
		 *
		 * Constructs an info object for the given user-visible text
		 * \em uvt, default key sequence list \em seqs and action icon
		 * \em icon.
		 */
		ActionInfo (const QString& uvt,
				const QKeySequences_t& seqs,
				const QIcon& icon)
		: UserVisibleText_ (uvt)
		, Seqs_ (seqs)
		, Icon_ (icon)
		{
		}
	};
};

Q_DECLARE_METATYPE (LC::ActionInfo)

/** @brief Interface for plugins that support configuring shortcuts.
 *
 * LC::Util::ShortcutManager class can help creating the
 * GetActionInfo() map and keeping track of created actions,
 * automatically updating their shortcuts.
 *
 * @sa LC::Util::ShortcutManager
 */
class Q_DECL_EXPORT IHaveShortcuts
{
public:
	/** @brief Sets shortcut's list of key sequences if it has been changed.
	 *
	 * The id is the same as in the return value of GetActionInfo().
	 *
	 * @param[in] id The id of the action.
	 * @param[in] sequences The new key sequences.
	 */
	virtual void SetShortcut (const QString& id, const QKeySequences_t& sequences) = 0;

	/** @brief Returns information about all the shortcuts.
	 *
	 * Returns a QMap from action id to the ActionInfo. Action id would
	 * be further used in SetShortcut and IShortcutProxy::GetShortcut(),
	 * for example.
	 *
	 * @return Shortcut IDs mapped to the corresponding ActionInfo.
	 */
	virtual QMap<QString, LC::ActionInfo> GetActionInfo () const = 0;

	virtual ~IHaveShortcuts () { }
};

Q_DECLARE_INTERFACE (IHaveShortcuts, "org.Deviant.LeechCraft.IHaveShortcuts/1.0")
