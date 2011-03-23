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
#include <QFileSystemWatcher>
#include <QtDebug>

namespace LeechCraft
{
	namespace Util
	{
		ResourceLoader::ResourceLoader (const QString& relPath, QObject* parent)
		: QObject (parent)
		, RelativePath_ (relPath)
		, SubElemModel_ (new QStandardItemModel (this))
		, AttrFilters_ (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable)
		, SortModel_ (new QSortFilterProxyModel (this))
		, Watcher_ (new QFileSystemWatcher (this))
		{
			if (RelativePath_.startsWith ('/'))
				RelativePath_ = RelativePath_.mid (1);
			if (!RelativePath_.endsWith ('/'))
				RelativePath_.append ('/');

			SortModel_->setDynamicSortFilter (true);
			SortModel_->setSourceModel (SubElemModel_);
			SortModel_->sort (0);

			connect (Watcher_,
					SIGNAL (directoryChanged (const QString&)),
					this,
					SLOT (handleDirectoryChanged (const QString&)));
		}

		void ResourceLoader::AddLocalPrefix (QString prefix)
		{
			if (!prefix.isEmpty () &&
					!prefix.endsWith ('/'))
				prefix.append ('/');
			QString result = QDir::homePath () + "/.leechcraft/data/" + prefix;
			LocalPrefixesChain_ << result;

			QDir testDir = QDir::home ();
			if (!testDir.exists (".leechcraft/data/" + prefix + RelativePath_))
			{
				qDebug () << Q_FUNC_INFO
						<< ".leechcraft/data/" + prefix + RelativePath_
						<< "doesn't exist, trying to create it...";

				if (!testDir.mkpath (".leechcraft/data/" + prefix + RelativePath_))
				{
					qWarning () << Q_FUNC_INFO
							<< "failed to create"
							<< ".leechcraft/data/" + prefix + RelativePath_;
				}
			}

			ScanPath (result + RelativePath_);

			Watcher_->addPath (result + RelativePath_);
		}

		void ResourceLoader::AddGlobalPrefix ()
		{
#ifdef Q_WS_MAC
			QStringList prefixes = QStringList (QApplication::applicationDirPath () + "/../Resources/");
#elif defined (Q_WS_WIN)
			QStringList prefixes = QStringList ("share/");
#elif defined (INSTALL_PREFIX)
			QStringList prefixes = QStringList (INSTALL_PREFIX "/share/leechcraft/");
#else
			QStringList prefixes = QStringList ("/usr/local/share/leechcraft/")
					<< "/usr/share/leechcraft/";
#endif
			Q_FOREACH (const QString& prefix, prefixes)
			{
				GlobalPrefixesChain_ << prefix;
				ScanPath (prefix + RelativePath_);

				if (!QFile::exists (prefix + RelativePath_))
					qWarning () << Q_FUNC_INFO
							<< prefix + RelativePath_
							<< "doesn't exist, not adding it";
				else
					Watcher_->addPath (prefix + RelativePath_);
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
		
		QString ResourceLoader::GetIconPath (const QString& basename) const
		{
			QStringList variants;
			variants << basename + ".svg"
					<< basename + ".png"
					<< basename + ".jpg"
					<< basename + ".gif";
			return GetPath (variants);
		}

		QIODevice_ptr ResourceLoader::Load (const QStringList& pathVariants) const
		{
			QString path = GetPath (pathVariants);
			if (path.isNull ())
				return QIODevice_ptr ();

			boost::shared_ptr<QFile> result (new QFile (path));
			return result;
		}
		
		QIODevice_ptr ResourceLoader::Load (const QString& pathVariant) const
		{
			return Load (QStringList (pathVariant));
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

		void ResourceLoader::handleDirectoryChanged (const QString& path)
		{
			QFileInfo fi (path);
			if (fi.exists () &&
					fi.isDir () &&
					fi.isReadable ())
				ScanPath (path);
		}
	}
}
