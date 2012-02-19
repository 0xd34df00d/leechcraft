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
#include <QMap>

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	namespace Proto
	{
		class Connection;
	}

	class MRIMAccount;
	class MRIMBuddy;

	class GroupManager : public QObject
	{
		Q_OBJECT

		MRIMAccount *A_;
		Proto::Connection *Conn_;

		QMap<int, QString> ID2Group_;
		QMap<QString, int> Group2ID_;

		QMap<quint32, QString> PendingGroups_;
		QMap<QString, QList<MRIMBuddy*>> PendingContacts_;
	public:
		GroupManager (MRIMAccount *parent);

		QString GetGroup (int) const;
		int GetGroupNumber (const QString&) const;
		void SetBuddyGroups (MRIMBuddy*, const QStringList&);
	private slots:
		void handleGotGroups (const QStringList&);
		void handleGroupAdded (quint32, quint32);
	};
}
}
}
