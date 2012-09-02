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

#include <memory>
#include <QHash>
#include <QIcon>

namespace LeechCraft
{
namespace Launchy
{
	class Item;

	typedef std::shared_ptr<Item> Item_ptr;

	class Item
	{
		QHash<QString, QString> Name_;
		QHash<QString, QString> GenericName_;
		QHash<QString, QString> Comments_;

		QStringList Categories_;
		QString Command_;
		QString WD_;

		QString IconName_;
		QIcon Icon_;
	public:
		enum class Type
		{
			Other,
			Application,
			URL,
			Dir
		};
	private:
		Type Type_;
	public:
		bool operator== (const Item&) const;

		bool IsValid () const;

		QString GetName (const QString&) const;
		QString GetGenericName (const QString&) const;
		QString GetComment (const QString&) const;
		QString GetIconName () const;
		QStringList GetCategories () const;

		Type GetType () const;
		QString GetCommand () const;
		QString GetWorkingDirectory () const;

		void SetIcon (const QIcon&);
		QIcon GetIcon () const;

		static Item_ptr FromDesktopFile (const QString&);
	};
}
}
