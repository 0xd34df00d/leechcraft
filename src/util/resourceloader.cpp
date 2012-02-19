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

#include "resourceloader.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QtDebug>
#include <QBuffer>

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
		, CacheFlushTimer_ (new QTimer (this))
		, CachePathContents_ (0)
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

			connect (CacheFlushTimer_,
					SIGNAL (timeout ()),
					this,
					SLOT (handleFlushCaches ()));
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
#ifdef Q_OS_MAC
			QStringList prefixes = QStringList (QApplication::applicationDirPath () + "/../Resources/");
#elif defined (Q_OS_WIN32)
			QStringList prefixes = QStringList (QApplication::applicationDirPath () + "/share/");
#elif defined (INSTALL_PREFIX)
			QStringList prefixes = QStringList (INSTALL_PREFIX "/share/leechcraft/");
#else
			QStringList prefixes = QStringList ("/usr/local/share/leechcraft/")
					<< "/usr/share/leechcraft/";
#endif
			bool hasBeenAdded = false;
			Q_FOREACH (const QString& prefix, prefixes)
			{
				GlobalPrefixesChain_ << prefix;
				ScanPath (prefix + RelativePath_);

				if (QFile::exists (prefix + RelativePath_))
				{
					Watcher_->addPath (prefix + RelativePath_);
					hasBeenAdded = true;
				}
			}

			if (!hasBeenAdded)
				qWarning () << Q_FUNC_INFO
						<< "no prefixes have been added:"
						<< prefixes
						<< "; rel path:"
						<< RelativePath_;
		}

		void ResourceLoader::SetCacheParams (int size, int timeout)
		{
			if (size <= 0)
			{
				CacheFlushTimer_->stop ();
				CachePathContents_.clear ();
			}
			else
			{
				if (timeout > 0)
					CacheFlushTimer_->start (timeout);

				CachePathContents_.setMaxCost (size * 1024);
			}
		}

		void ResourceLoader::FlushCache ()
		{
			CachePathContents_.clear ();
		}

		QFileInfoList ResourceLoader::List (const QString& option,
				const QStringList& nameFilters, QDir::Filters filters) const
		{
			QSet<QString> alreadyListed;
			QFileInfoList result;
			Q_FOREACH (const QString& prefix,
					LocalPrefixesChain_ + GlobalPrefixesChain_)
			{
				const QString& path = prefix + RelativePath_ + option;
				QDir dir (path);
				const QFileInfoList& list =
						dir.entryInfoList (nameFilters, filters);
				Q_FOREACH (const QFileInfo& info, list)
				{
					const QString& fname = info.fileName ();
					if (alreadyListed.contains (fname))
						continue;

					alreadyListed << fname;
					result << info;
				}
			}

			return result;
		}

		QString ResourceLoader::GetPath (const QStringList& pathVariants) const
		{
			Q_FOREACH (const QString& prefix,
					LocalPrefixesChain_ + GlobalPrefixesChain_)
				Q_FOREACH (const QString& path, pathVariants)
				{
					const QString& can = QFileInfo (prefix + RelativePath_ + path).canonicalFilePath ();
					if (QFile::exists (can))
						return can;
				}

			return QString ();
		}

		namespace
		{
			QStringList IconizeBasename (const QString& basename)
			{
				QStringList variants;
				variants << basename + ".svg"
						<< basename + ".png"
						<< basename + ".jpg"
						<< basename + ".gif";
				return variants;
			}
		}

		QString ResourceLoader::GetIconPath (const QString& basename) const
		{
			return GetPath (IconizeBasename (basename));
		}

		QIODevice_ptr ResourceLoader::Load (const QStringList& pathVariants, bool open) const
		{
			QString path = GetPath (pathVariants);
			if (path.isNull ())
				return QIODevice_ptr ();

			if (CachePathContents_.contains (path))
			{
				std::shared_ptr<QBuffer> result (new QBuffer ());
				result->setData (*CachePathContents_ [path]);
				if (open)
					result->open (QIODevice::ReadOnly);
				return result;
			}

			std::shared_ptr<QFile> result (new QFile (path));

			if (!result->isSequential () &&
					result->size () < CachePathContents_.maxCost () / 2)
			{
				if (result->open (QIODevice::ReadOnly))
				{
					const QByteArray& data = result->readAll ();
					CachePathContents_.insert (path, new QByteArray (data), data.size ());
					result->close ();
				}
			}

			if (open)
				result->open (QIODevice::ReadOnly);

			return result;
		}

		QIODevice_ptr ResourceLoader::Load (const QString& pathVariant, bool open) const
		{
			return Load (QStringList (pathVariant), open);
		}

		QIODevice_ptr ResourceLoader::LoadIcon (const QString& basename, bool open) const
		{
			return Load (IconizeBasename (basename), open);
		}

		QPixmap ResourceLoader::LoadPixmap (const QString& basename) const
		{
			auto dev = LoadIcon (basename, true);
			if (!dev)
				return QPixmap ();

			QPixmap px;
			px.loadFromData (dev->readAll ());
			return px;
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
			emit watchedDirectoriesChanged ();

			QFileInfo fi (path);
			if (fi.exists () &&
					fi.isDir () &&
					fi.isReadable ())
				ScanPath (path);
		}

		void ResourceLoader::handleFlushCaches ()
		{
			CachePathContents_.clear ();
		}
	}
}
