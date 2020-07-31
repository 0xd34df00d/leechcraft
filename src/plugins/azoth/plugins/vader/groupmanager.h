/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>

namespace LC
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
