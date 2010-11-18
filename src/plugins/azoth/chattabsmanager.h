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

#ifndef PLUGINS_AZOTH_CHATTABSMANAGER_H
#define PLUGINS_AZOTH_CHATTABSMANAGER_H
#include <QObject>
#include <QHash>
#include <QPersistentModelIndex>
#include "chattab.h"

class QWidget;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class ICLEntry;
			}

			class ChatTab;

			class ChatTabsManager : public QObject
			{
				Q_OBJECT

				QHash<Plugins::ICLEntry*, ChatTab_ptr> Entry2Tab_;
			public:
				ChatTabsManager(QObject* = 0);

				void OpenChat (const QModelIndex&);
				bool IsActiveChat (Plugins::ICLEntry*) const;
			private slots:
				void handleNeedToClose (ChatTab*);
			signals:
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void raiseTab (QWidget*);
			};
		}
	}
}

#endif
