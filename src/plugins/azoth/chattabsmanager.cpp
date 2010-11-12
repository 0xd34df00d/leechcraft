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

#include "chattabsmanager.h"
#include <QtDebug>
#include "interfaces/iclentry.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			ChatTabsManager::ChatTabsManager(QObject *parent)
			: QObject (parent)
			{
			}

			void ChatTabsManager::OpenChat (const QModelIndex& ti)
			{
				if (!ti.isValid ())
				{
					qWarning () << Q_FUNC_INFO
							<< "tried to open a chat with invalid index";
					return;
				}

				QPersistentModelIndex idx (ti);
				if (Index2Tab_.contains (idx))
				{
					emit raiseTab (Index2Tab_ [idx]);
					return;
				}

				QObject *entryObj = ti.data (Core::CLREntryObject).value<QObject*> ();
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (entryObj);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< "object"
							<< entryObj
							<< "from the index"
							<< ti
							<< "doesn't implement Plugins::ICLEntry";
					return;
				}

				// TODO don't hardcode the first variant
				QPointer<ChatTab> tab (new ChatTab (idx, entry->Variants ().first ()));
				Index2Tab_ [idx] = tab;
				emit addNewTab (entry->GetEntryName(), tab);
				emit raiseTab (tab);
			}
		}
	}
}
