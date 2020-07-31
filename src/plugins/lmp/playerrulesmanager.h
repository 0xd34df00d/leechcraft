/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace LC
{
struct Entity;

namespace LMP
{
	class PlayerRulesManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;

		QList<QStandardItem*> ManagedItems_;

		QList<Entity> Rules_;
	public:
		PlayerRulesManager (QStandardItemModel*, QObject* = 0);

		void InitializePlugins ();
	private slots:
		void insertRows (const QModelIndex&, int, int);
		void removeRows (const QModelIndex&, int, int);
		void handleReset ();

		void refillRules ();

		void handleRulesChanged ();
	};
}
}
