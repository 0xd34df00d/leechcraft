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
#include "xmlsettingsmanager.h"

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

				const QByteArray& id = entry->GetEntryID ();
				if (Entry2Tab_.contains (id))
				{
					emit raiseTab (Entry2Tab_ [id]);
					return;
				}

				// TODO don't hardcode the first variant
				QPointer<ChatTab> tab (new ChatTab (id, entry->Variants ().first ()));
				Entry2Tab_ [id] = tab;

				connect (tab,
						SIGNAL (needToClose (ChatTab*)),
						this,
						SLOT (handleNeedToClose (ChatTab*)));
				connect (tab,
						SIGNAL (clearUnreadMsgCount (QObject*)),
						this,
						SIGNAL (clearUnreadMsgCount (QObject*)));
				connect (tab,
						SIGNAL (changeTabName (QWidget*, const QString&)),
						this,
						SIGNAL (changeTabName (QWidget*, const QString&)));
				connect (tab,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
						this,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)));

				emit addNewTab (entry->GetEntryName (), tab);

				tab->HasBeenAdded ();

				if (XmlSettingsManager::Instance ()
						.property ("JumpToNewTabOnOpen").toBool ())
					emit raiseTab (tab);
			}

			bool ChatTabsManager::IsActiveChat (const Plugins::ICLEntry *entry) const
			{
				if (!Entry2Tab_.contains (entry->GetEntryID ()))
					return false;

				return Entry2Tab_ [entry->GetEntryID ()]->isVisible ();
			}

			void ChatTabsManager::UpdateEntryMapping (const QByteArray& id, QObject *obj)
			{
				if (!Entry2Tab_.contains (id))
					return;

				connect (obj,
						SIGNAL (gotMessage (QObject*)),
						Entry2Tab_ [id],
						SLOT (handleEntryMessage (QObject*)),
						Qt::UniqueConnection);
			}

			void ChatTabsManager::SetChatEnabled (const QByteArray& id, bool enabled)
			{
				if (!Entry2Tab_.contains (id))
					return;

				Entry2Tab_ [id]->setEnabled (enabled);
			}

			void ChatTabsManager::handleNeedToClose (ChatTab *tab)
			{
				emit removeTab (tab);

				QByteArray entry = Entry2Tab_.key (tab);
				Entry2Tab_.remove (entry);

				tab->deleteLater ();
			}
		}
	}
}
