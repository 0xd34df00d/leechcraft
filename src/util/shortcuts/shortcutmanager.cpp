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

#include "shortcutmanager.h"
#include <QAction>
#include <QShortcut>
#include "interfaces/ihaveshortcuts.h"

namespace LeechCraft
{
namespace Util
{
	ShortcutManager::ShortcutManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, CoreProxy_ (proxy)
	, ContextObj_ (0)
	{
	}

	void ShortcutManager::SetObject (QObject *obj)
	{
		ContextObj_ = obj;
	}

	void ShortcutManager::RegisterAction (const QString& id, QAction *act, bool update)
	{
		Actions_ [id] << act;
		connect (act,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleActionDestroyed ()));

		const QIcon& icon = act->icon ().isNull () ?
				CoreProxy_->GetIcon (act->property ("ActionIcon").toString ()) :
				act->icon ();
		RegisterActionInfo (id,
				{ act->text (), act->shortcuts (), icon });

		if (update)
			SetShortcut (id,
					CoreProxy_->GetShortcutProxy ()->GetShortcuts (ContextObj_, id));
	}

	void ShortcutManager::RegisterShortcut (const QString& id, const ActionInfo& info, QShortcut* shortcut, bool update)
	{
		Shortcuts_ [id] << shortcut;
		connect (shortcut,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleShortcutDestroyed ()));

		RegisterActionInfo (id, info);

		if (update)
			SetShortcut (id,
					CoreProxy_->GetShortcutProxy ()->GetShortcuts (ContextObj_, id));
	}

	void ShortcutManager::RegisterActionInfo (const QString& id, const ActionInfo& info)
	{
		if (!ActionInfo_.contains (id))
			ActionInfo_ [id] = info;
	}

	void ShortcutManager::SetShortcut (const QString& id, const QKeySequences_t& seqs) const
	{
		for (auto act : Actions_ [id])
			act->setShortcuts (seqs);

		for (auto sc : Shortcuts_ [id])
			sc->setKey (seqs.value (0));
	}

	QMap<QString, ActionInfo> ShortcutManager::GetActionInfo () const
	{
		return ActionInfo_;
	}

	ShortcutManager& ShortcutManager::operator<< (const QPair<QString, QAction*>& pair)
	{
		RegisterAction (pair.first, pair.second);
		return *this;
	}

	void ShortcutManager::handleActionDestroyed ()
	{
		auto act = static_cast<QAction*> (sender ());
		for (const auto& id : Actions_.keys ())
			Actions_ [id].removeAll (act);
	}

	void ShortcutManager::handleShortcutDestroyed()
	{
		auto sc = static_cast<QShortcut*> (sender ());
		for (const auto& id : Shortcuts_.keys ())
			Shortcuts_ [id].removeAll (sc);
	}
}
}
