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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_GROUPMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_GROUPMANAGER_H
#include <QObject>
#include <QHash>
#include <QStringList>
#include <msn/buddy.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class Callbacks;
	class MSNAccount;
	class MSNBuddyEntry;

	class GroupManager : public QObject
	{
		Q_OBJECT

		MSNAccount *Account_;
		Callbacks *CB_;

		QHash<QString, QString> Group2ID_;
		QHash<QString, QString> ID2Group_;

		QHash<QString, QStringList> PendingAdditions_;
	public:
		GroupManager (Callbacks*, MSNAccount*);

		void SetGroups (MSNBuddyEntry*,
				const QStringList& newGroups, const QStringList& oldGroups);
	private:
		void AddGroup (const QString&, const QString&);
		void RemoveGroup (const QString&, const QString&);
	private slots:
		void handleGotGroups (const QList<MSN::Group>&);
		void handleRemovedGroup (const QString&);
		void handleRenamedGroup (const QString&, const QString&);

		void handleBuddyAdded (const QString&, const QString&);
		void handleBuddyRemoved (const QString&, const QString&);
	};
}
}
}

#endif
