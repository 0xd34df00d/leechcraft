/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "resourceloader.h"
#include <QFile>
#include <QDir>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QtDebug>

namespace LeechCraft
{
	namespace Util
	{
		ResourceLoader::ResourceLoader (const QString& relPath, QObject* parent)
		: QObject (parent)
		, RelativePath_ (relPath)
		, SubElemModel_ (new QStandardItemModel (this))
		, SortModel_ (new QSortFilterProxyModel (this))
		, AttrFilters_ (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable)
		{
			if (RelativePath_.startsWith ('/'))
				RelativePath_ = RelativePath_.mid (1);
			if (!RelativePath_.endsWith ('/'))
				RelativePath_.append ('/');

			SortModel_->setDynamicSortFilter (true);
			SortModel_->setSourceModel (SubElemModel_);
			SortModel_->sort (0);
		}

		void ResourceLoader::AddLocalPrefix (QString prefix)
		{
			if (!prefix.isEmpty () &&
					!prefix.endsWith ('/'))
				prefix.append ('/');
			QString result = QDir::homePath () + "/.leechcraft/data/" + prefix;
			LocalPrefixesChain_ << result;

			ScanPath (result + RelativePath_);
		}

		void ResourceLoader::AddGlobalPrefix ()
		{
#ifdef Q_WS_MAC
			QStringList prefixes = QStirngList (QApplication::applicationDirPath () + "/../Resources/");
#elif defined Q_WS_WIN
			QStringList prefixes = QStringList ("share/");
#else
			QStringList prefixes = QStringList ("/usr/local/share/leechcraft/")
					<< "/usr/share/leechcraft/";
#endif
			Q_FOREACH (const QString& prefix, prefixes)
			{
				GlobalPrefixesChain_ << prefix;
				ScanPath (prefix + RelativePath_);
			}
		}

		QString ResourceLoader::GetPath (const QStringList& pathVariants) const
		{
			Q_FOREACH (const QString& prefix,
					LocalPrefixesChain_ + GlobalPrefixesChain_)
				Q_FOREACH (const QString& path, pathVariants)
					if (QFile::exists (prefix + RelativePath_ + path))
						return prefix + RelativePath_ + path;

			return QString ();
		}

		QIODevice_ptr ResourceLoader::Load (const QStringList& pathVariants) const
		{
			QString path = GetPath (pathVariants);
			if (path.isNull ())
				return QIODevice_ptr ();

			boost::shared_ptr<QFile> result (new QFile (path));
			return result;
		}

		QAbstractItemModel* ResourceLoader::GetSubElemModel () const
		{
			return SortModel_;
		}

		void ResourceLoader::SetAttrFilters (QDir::Filters filters)
		{
			AttrFilters_ = filters;
		}

		void ResourceLoader::SetNameFilters (const QStringList& filters)
		{
			NameFilters_ = filters;
		}

		void ResourceLoader::ScanPath (const QString& path)
		{
			Q_FOREACH (const QString& entry,
					QDir (path).entryList (NameFilters_, AttrFilters_))
			{
				if (SubElemModel_->findItems (entry).size ())
					continue;

				SubElemModel_->appendRow (new QStandardItem (entry));
			}
		}
	}
}
