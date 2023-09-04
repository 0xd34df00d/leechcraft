/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QtPlugin>
#include <QString>
#include <QKeySequence>
#include <QIcon>
#include <QMetaType>
#include <util/sll/void.h>

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
		QString Text_ {};
		/// The primary key sequence for this action.
		QKeySequence Seq_ {};
		/// Icon of the action, either a `QIcon` itself or its `ActionIcon`.
		std::variant<Util::Void, QByteArray, QIcon> Icon_ { Util::Void {} };
		/// The additional key sequences for this action.
		QKeySequences_t AdditionalSeqs_ {};

		QKeySequences_t GetAllShortcuts () const
		{
			if (AdditionalSeqs_.isEmpty ())
				return { Seq_ };

			QKeySequences_t result;
			result.reserve (AdditionalSeqs_.size () + 1);
			result.push_back (Seq_);
			result += AdditionalSeqs_;
			return result;
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
	virtual void SetShortcut (const QByteArray& id, const QKeySequences_t& sequences) = 0;

	/** @brief Returns information about all the shortcuts.
	 *
	 * Returns a QMap from action id to the ActionInfo. Action id would
	 * be further used in SetShortcut and IShortcutProxy::GetShortcut(),
	 * for example.
	 *
	 * @return Shortcut IDs mapped to the corresponding ActionInfo.
	 */
	virtual QMap<QByteArray, LC::ActionInfo> GetActionInfo () const = 0;

	virtual ~IHaveShortcuts () { }
};

Q_DECLARE_INTERFACE (IHaveShortcuts, "org.Deviant.LeechCraft.IHaveShortcuts/1.0")
