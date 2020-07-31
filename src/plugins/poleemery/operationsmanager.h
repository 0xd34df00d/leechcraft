/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QSet>
#include "structures.h"

class QStandardItemModel;
class QAbstractItemModel;
class QModelIndex;

namespace LC
{
namespace Poleemery
{
	class Storage;
	typedef std::shared_ptr<Storage> Storage_ptr;

	class EntriesModel;

	class OperationsManager : public QObject
	{
		Q_OBJECT

		const Storage_ptr Storage_;
		EntriesModel *Model_;

		QSet<QString> KnownCategories_;
	public:
		OperationsManager (Storage_ptr, QObject* = 0);

		void Load ();

		QAbstractItemModel* GetModel () const;

		QList<EntryBase_ptr> GetAllEntries () const;
		QList<EntryWithBalance> GetEntriesWBalance () const;

		QSet<QString> GetKnownCategories () const;

		void AddEntry (EntryBase_ptr);
		void UpdateEntry (EntryBase_ptr);
		void RemoveEntry (const QModelIndex&);
	};
}
}
