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
#include <QList>

class QAction;
class QStringList;

namespace LC::Azoth::Autopaste
{
	class ActionsStorage : public QObject
	{
		Q_OBJECT

		QHash<QObject*, QList<QAction*>> Entry2Actions_;
	public:
		using QObject::QObject;

		QList<QAction*> GetEntryActions (QObject*);
		QStringList GetActionAreas (const QObject*) const;
	signals:
		void pasteRequested (QObject*);
	};
}
