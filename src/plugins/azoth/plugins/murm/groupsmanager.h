/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QSet>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkConnection;
	struct ListInfo;
	struct UserInfo;

	class GroupsManager : public QObject
	{
		Q_OBJECT

		VkConnection * const Conn_;
		QHash<qulonglong, ListInfo> ID2ListInfo_;

		QHash<qulonglong, QSet<qulonglong>> List2IDs_;

		QSet<qulonglong> ModifiedLists_;
		QHash<QString, QSet<qulonglong>> NewLists_;

		bool IsApplyScheduled_ = false;
	public:
		GroupsManager (VkConnection*);

		ListInfo GetListInfo (qulonglong) const;
		ListInfo GetListInfo (const QString& name) const;

		void UpdateGroups (const QStringList& oldGroups, const QStringList& newGroups, qulonglong id);
	private slots:
		void applyChanges ();
	public slots:
		void handleLists (const QList<ListInfo>&);
		void handleAddedLists (const QList<ListInfo>&);
		void handleUsers (const QList<UserInfo>&);
	};
}
}
}
