/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef INTERFACES_IHAVESHORTCUTS_H
#define INTERFACES_IHAVESHORTCUTS_H
#include <QtPlugin>
#include <QMultiMap>
#include <QString>
#include <QKeySequence>
#include <QIcon>
#include <QMetaType>

class QAction;

typedef QList<QKeySequence> QKeySequences_t;
Q_DECLARE_METATYPE (QKeySequences_t);

namespace LeechCraft
{
	/** Contains information about the action that could be handled by
	 * the shortcut manager.
	 */
	struct ActionInfo
	{
		/// User-visible name of the action.
		QString UserVisibleText_;
		/// List of key sequences for this action.
		QKeySequences_t Seqs_;
		/// Icon of the action.
		QIcon Icon_;

		ActionInfo ()
		{
		}

		ActionInfo (const QString& uvt,
				const QKeySequence& seq,
				const QIcon& icon)
		: UserVisibleText_ (uvt)
		, Icon_ (icon)
		{
			Seqs_ << seq;
		}

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

Q_DECLARE_METATYPE (LeechCraft::ActionInfo);

/** @brief Proxy for requesting shortcuts from the shortcut manager in
 * the Core.
 *
 * The plugin can communicate with the shortcut manager via this proxy.
 *
 * @sa IHaveShortcuts::SetShortcutProxy().
 */
class IShortcutProxy
{
public:
	/** @brief Returns a QKeySequence for the given action.
	 *
	 * Returns a list of key sequences for the action with given id for
	 * the given object which is currently set in the shortcut manager.
	 * The id is the same as in return value of
	 * IHaveShortcuts::GetActionInfo().
	 *
	 * The object is used to distinguish between ids of different
	 * plugins. It can be said that object defines the context for the
	 * id.
	 *
	 * @param[in] object The object that should be checked.
	 * @param[in] id ID of the action.
	 * @return The key sequences for the passed action.
	 */
	virtual QKeySequences_t GetShortcuts (const QObject *object, const QString& id) const = 0;

	virtual ~IShortcutProxy () { }
};

class IHaveShortcuts
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
	virtual QMap<QString, LeechCraft::ActionInfo> GetActionInfo () const = 0;

	virtual ~IHaveShortcuts () { }
};

Q_DECLARE_INTERFACE (IShortcutProxy, "org.Deviant.LeechCraft.IShortcutProxy/1.0");
Q_DECLARE_INTERFACE (IHaveShortcuts, "org.Deviant.LeechCraft.IHaveShortcuts/1.0");

#endif

