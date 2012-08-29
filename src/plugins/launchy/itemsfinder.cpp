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
	{
		QTimer::singleShot (1000, this, SLOT (update ()));
	}

	QHash<QString, QList<Item_ptr>> ItemsFinder::GetItems () const
	{
		return Items_;
	}

	namespace
	{
		QStringList ScanDir (const QString& path)
		{
			const auto& infos = QDir (path).entryInfoList (QStringList ("*.desktop"));
			QStringList result;
			for (const auto& info : infos)
				result << info.absoluteFilePath ();
			return result;
		}

		QIcon LoadIcon (ICoreProxy_ptr proxy, QString name)
		{
			if (name.isEmpty ())
				return QIcon ();

			qDebug () << name;

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
		qDebug () << Q_FUNC_INFO;
		Items_.clear ();
		auto paths = ScanDir ("/usr/share/applications");
		paths += ScanDir ("/usr/share/applications/kde4");
		qDebug () << "scanned";

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

		qDebug () << Items_.keys ();
		qDebug () << "done";

		emit itemsListChanged ();
	}
}
}
