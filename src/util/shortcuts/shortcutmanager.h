/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QHash>
#include "interfaces/ihaveshortcuts.h"
#include "interfaces/core/icoreproxy.h"
#include "../utilconfig.h"

class QAction;
class IShortcutProxy;

namespace LeechCraft
{
namespace Util
{
	class ShortcutManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr CoreProxy_;
		QObject *ContextObj_;

		QHash<QString, QList<QAction*>> Actions_;

		QMap<QString, ActionInfo> ActionInfo_;
	public:
		UTIL_API ShortcutManager (ICoreProxy_ptr, QObject* = 0);

		UTIL_API void SetObject (QObject*);
		UTIL_API void RegisterAction (const QString& id, QAction *act, bool update = false);
		UTIL_API void RegisterActionInfo (const QString& id, const ActionInfo&);
		UTIL_API void SetShortcut (const QString& id, const QKeySequences_t&);
		UTIL_API QMap<QString, ActionInfo> GetActionInfo () const;

		typedef QPair<QString, QAction*> IDPair_t;
		UTIL_API ShortcutManager& operator<< (const QPair<QString, QAction*>&);
	private slots:
		void handleActionDestroyed ();
	};
}
}
