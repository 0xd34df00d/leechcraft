/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "itemsfinder.h"
#include <QDir>
#include <QTimer>
#include <QtDebug>
#include "item.h"

namespace LeechCraft
{
namespace Launchy
{
	ItemsFinder::ItemsFinder (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, IsReady_ (false)
	{
		QTimer::singleShot (1000, this, SLOT (update ()));
	}

	bool ItemsFinder::IsReady () const
	{
		return IsReady_;
	}

	QHash<QString, QList<Item_ptr>> ItemsFinder::GetItems () const
	{
		return Items_;
	}

	Item_ptr ItemsFinder::FindItem (const QString& id) const
	{
		for (const auto& list : Items_)
		{
			const auto pos = std::find_if (list.begin (), list.end (),
					[&id] (Item_ptr item) { return item->GetPermanentID () == id; });
			if (pos != list.end ())
				return *pos;
		}

		return Item_ptr ();
	}

	namespace
	{
		QStringList ScanDir (const QString& path)
		{
			const auto& infos = QDir (path).entryInfoList (QStringList ("*.desktop"),
						QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
			QStringList result;
			for (const auto& info : infos)
				result += info.isDir () ?
						ScanDir (info.absoluteFilePath ()) :
						QStringList (info.absoluteFilePath ());
			return result;
		}

		QIcon LoadIcon (ICoreProxy_ptr proxy, QString name)
		{
			if (name.isEmpty ())
				return QIcon ();

			if (name.endsWith (".png") || name.endsWith (".svg"))
				name.chop (4);

			auto result = proxy->GetIcon (name);
			if (!result.isNull ())
				return result;

			for (auto ext : { ".png", ".svg", ".xpm", ".jpg" })
			{
				if (QFile::exists ("/usr/share/pixmaps/" + name + ext))
					return QIcon ("/usr/share/pixmaps/" + name + ext);
				if (QFile::exists ("/usr/local/share/pixmaps/" + name + ext))
					return QIcon ("/usr/local/share/pixmaps/" + name + ext);
			}

			if (result.isNull ())
				qDebug () << Q_FUNC_INFO << name << "not found";

			return result;
		}
	}

	void ItemsFinder::update ()
	{
		IsReady_ = true;

		Items_.clear ();
		auto paths = ScanDir ("/usr/share/applications");

		for (const auto& path : paths)
		{
			Item_ptr item;
			try
			{
				item = Item::FromDesktopFile (path);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error parsing"
						<< path
						<< e.what ();
				continue;
			}
			if (!item->IsValid ())
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid item"
						<< path;
				continue;
			}

			item->SetIcon (LoadIcon (Proxy_, item->GetIconName ()));

			for (const auto& cat : item->GetCategories ())
				if (!cat.startsWith ("X-"))
					Items_ [cat] << item;
		}

		emit itemsListChanged ();
	}
}
}
