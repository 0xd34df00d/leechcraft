/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_LACKMAN_PENDINGMANAGER_H
#define PLUGINS_LACKMAN_PENDINGMANAGER_H
#include <QObject>
#include <QSet>
#include <QMap>

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			class PendingManager : public QObject
			{
				Q_OBJECT

				QStandardItemModel *PendingModel_;

				QSet<int> ScheduledInstall_;
				QSet<int> ScheduledRemove_;
				QSet<int> ScheduledUpdate_;

				QMap<int, QList<int> > Deps_;
				QMap<int, QStandardItem*> ID2ModelRow_;
			public:
				PendingManager (QObject* = 0);

				QAbstractItemModel* GetPendingModel () const;
				void Reset ();
				void ToggleInstallRemove (int id, bool enable, bool installed);
				void ToggleUpdate (int id, bool enable);
			private:
				void EnablePackageInto (int, QSet<int>&);
				void DisablePackageFrom (int, QSet<int>&);
			};
		}
	}
}

#endif
