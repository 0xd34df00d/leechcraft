/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "todostorage.h"
#include <algorithm>
#include <QCoreApplication>
#include <QtDebug>
#include <util/sll/prelude.h>

namespace LC
{
namespace Otlozhu
{
	TodoStorage::TodoStorage (const QString& ctx, QObject *parent)
	: QObject (parent)
	, Context_ (ctx)
	, Storage_ (QSettings::IniFormat,
			QSettings::UserScope,
			QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Otlozhu_" + ctx)
	{
		Load ();
	}

	int TodoStorage::GetNumItems () const
	{
		return Items_.size ();
	}

	int TodoStorage::FindItem (const QString& id) const
	{
		const auto pos = std::find_if (Items_.begin (), Items_.end (),
				[&id] (const auto& item) { return item->GetID () == id; });
		return pos == Items_.end () ?
				-1 :
				std::distance (Items_.begin (), pos);
	}

	void TodoStorage::AddItem (TodoItem_ptr item)
	{
		Items_ << item;
		SaveAt (Items_.size () - 1);
		emit itemAdded (Items_.size () - 1);
	}

	TodoItem_ptr TodoStorage::GetItemAt (int idx) const
	{
		return Items_ [idx]->Clone ();
	}

	TodoItem_ptr TodoStorage::GetItemByID (const QString& id) const
	{
		const auto pos = FindItem (id);
		return pos == -1 ? nullptr : GetItemAt (pos);
	}

	QList<TodoItem_ptr> TodoStorage::GetAllItems () const
	{
		return Util::Map (Items_, [] (const auto& item) { return item->Clone (); });
	}

	void TodoStorage::AddDependency (const QString& itemId, const QString& depId)
	{
		if (depId.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot add an empty dep ID";
		}

		auto item = GetItemByID (itemId);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot find item"
					<< itemId;
			return;
		}

		item = item->Clone ();
		item->AddDep (depId);

		HandleUpdated (item,
				[this, itemId, item]
				{
					emit itemDepAdded (FindItem (itemId), item->GetDeps ().size () - 1);
				});
	}

	void TodoStorage::HandleUpdated (TodoItem_ptr item)
	{
		HandleUpdated (item, {});
	}

	void TodoStorage::RemoveItem (const QString& id)
	{
		const int pos = FindItem (id);
		if (pos == -1)
			return;

		for (int i = 0; i < Items_.size (); ++i)
		{
			const auto& item = Items_.at (i)->Clone ();
			auto deps = item->GetDeps ();
			const auto depIdx = deps.indexOf (id);
			if (depIdx < 0)
				continue;

			deps.removeAt (depIdx);
			item->SetDeps (deps);

			HandleUpdated (item, [this, i, depIdx] { emit itemDepRemoved (i, depIdx); });
		}

		emit itemRemoved (pos);
		Items_.removeAt (pos);

		QList<int> indexes;
		for (int i = pos; i < GetNumItems (); ++i)
			indexes << i;
		SaveAt (indexes);
	}

	void TodoStorage::HandleUpdated (TodoItem_ptr item, const std::function<void ()>& preEmit)
	{
		const int pos = FindItem (item->GetID ());
		if (pos == -1)
			return;

		emit itemDiffGenerated (item->GetID (), item->DiffWith (Items_ [pos]));
		Items_ [pos] = item;

		if (preEmit)
			preEmit ();

		emit itemUpdated (pos);
		SaveAt (pos);
	}

	void TodoStorage::Load ()
	{
		Items_.clear ();

		Storage_.beginGroup ("Items");
		const int size = Storage_.beginReadArray ("List");
		for (int i = 0; i < size; ++i)
		{
			Storage_.setArrayIndex (i);
			TodoItem_ptr item = TodoItem::Deserialize (Storage_.value ("Item").toByteArray ());
			Items_ << item;
			emit itemAdded (i);
		}
		Storage_.endArray ();
		Storage_.endGroup ();
	}

	void TodoStorage::SaveAt (int idx)
	{
		Storage_.beginGroup ("Items");
		Storage_.beginWriteArray ("List", GetNumItems ());
		Storage_.setArrayIndex (idx);
		Storage_.setValue ("Item", GetItemAt (idx)->Serialize ());
		Storage_.endArray ();
		Storage_.endGroup ();
	}

	void TodoStorage::SaveAt (const QList<int>& indexes)
	{
		Storage_.beginGroup ("Items");
		Storage_.beginWriteArray ("List", GetNumItems ());
		for (int idx : indexes)
		{
			Storage_.setArrayIndex (idx);
			Storage_.setValue ("Item", GetItemAt (idx)->Serialize ());
		}
		Storage_.endArray ();
		Storage_.endGroup ();
	}
}
}
