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

#include "todostorage.h"
#include <QCoreApplication>

namespace LeechCraft
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

	void TodoStorage::AddItem (TodoItem_ptr item)
	{
		Items_ << item;
		SaveAt (Items_.size () - 1);
		emit itemAdded (Items_.size () - 1);
	}

	TodoItem_ptr TodoStorage::GetItemAt (int idx) const
	{
		return Items_ [idx];
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
		Storage_.beginWriteArray ("List");
		Storage_.setArrayIndex (idx);
		Storage_.setValue ("Item", GetItemAt (idx)->Serialize ());
		Storage_.endArray ();
		Storage_.endGroup ();
	}
}
}
