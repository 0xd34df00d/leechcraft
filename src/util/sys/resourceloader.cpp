/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC::Util
{
	ResourceLoader::ResourceLoader (const QString& relPath, QObject* parent)
	: QObject (parent)
	, RelativePath_ (relPath)
	, SubElemModel_ (new QStandardItemModel (this))
	, SortModel_ (new QSortFilterProxyModel (this))
	, Watcher_ (new QFileSystemWatcher (this))
	, CacheFlushTimer_ (new QTimer (this))
	{
		if (RelativePath_.startsWith ('/'))
			RelativePath_ = RelativePath_.mid (1);
		if (!RelativePath_.endsWith ('/'))
			RelativePath_.append ('/');

		SortModel_->setDynamicSortFilter (true);
		SortModel_->setSourceModel (SubElemModel_);
		SortModel_->sort (0);

		connect (Watcher_,
				&QFileSystemWatcher::directoryChanged,
				this,
				&ResourceLoader::HandleDirectoryChanged);

		connect (CacheFlushTimer_,
				&QTimer::timeout,
				this,
				&ResourceLoader::FlushCache);
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
#if defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
		const QStringList prefixes { QApplication::applicationDirPath () + "/../Resources/share/" };
#elif defined (Q_OS_WIN32)
		const QStringList prefixes { QApplication::applicationDirPath () + "/share/" };
#elif defined (INSTALL_PREFIX)
		const QStringList prefixes { INSTALL_PREFIX "/share/leechcraft/" };
#else
		const QStringList prefixes
		{
			"/usr/local/share/leechcraft/",
			"/usr/share/leechcraft/"
		};
#endif
		bool hasBeenAdded = false;
		for (const auto& prefix : prefixes)
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
		if (qApp->property ("no-resource-caching").toBool ())
			return;

		if (size <= 0)
		{
			CacheFlushTimer_->stop ();

			CachePathContents_.setMaxCost (0);
			CachePixmaps_.setMaxCost (0);
		}
		else
		{
			if (timeout > 0)
				CacheFlushTimer_->start (timeout);

			CachePathContents_.setMaxCost (size * 1024);
			CachePixmaps_.setMaxCost (size * 1024);
		}
	}

	void ResourceLoader::FlushCache ()
	{
		CachePathContents_.clear ();
		CachePixmaps_.clear ();
	}

	QFileInfoList ResourceLoader::List (const QString& option,
			const QStringList& nameFilters, QDir::Filters filters) const
	{
		QSet<QString> alreadyListed;
		QFileInfoList result;
		for (const auto& prefix : LocalPrefixesChain_ + GlobalPrefixesChain_)
		{
			const QDir dir { prefix + RelativePath_ + option };
			const auto& list = dir.entryInfoList (nameFilters, filters);
			for (const auto& info : list)
			{
				const auto& fname = info.fileName ();
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
		for (const auto& prefix : LocalPrefixesChain_ + GlobalPrefixesChain_)
			for (const auto& path : pathVariants)
			{
				if (Verbose_)
					qDebug () << Q_FUNC_INFO
							<< "trying"
							<< prefix + RelativePath_ + path;
				const QString& can = QFileInfo (prefix + RelativePath_ + path).absoluteFilePath ();
				if (Verbose_)
					qDebug () << Q_FUNC_INFO
							<< "absolute file path"
							<< can
							<< "; file exists?"
							<< QFile::exists (can);
				if (QFile::exists (can))
					return can;
			}

		return QString ();
	}

	namespace
	{
		QStringList IconizeBasename (const QString& basename)
		{
			return
			{
				basename + ".svg",
				basename + ".png",
				basename + ".jpg",
				basename + ".gif"
			};
		}
	}

	QString ResourceLoader::GetIconPath (const QString& basename) const
	{
		return GetPath (IconizeBasename (basename));
	}

	QIODevice_ptr ResourceLoader::Load (const QStringList& pathVariants, bool open) const
	{
		const auto& path = GetPath (pathVariants);
		if (path.isNull ())
			return {};

		if (CachePathContents_.contains (path))
		{
			if (Verbose_)
				qDebug () << Q_FUNC_INFO
						<< "found"
						<< path
						<< "in cache";

			auto result = std::make_shared<QBuffer> ();
			result->setData (*CachePathContents_ [path]);
			if (open)
				result->open (QIODevice::ReadOnly);
			return result;
		}

		auto result = std::make_shared<QFile> (path);

		if (!result->isSequential () &&
				result->size () < CachePathContents_.maxCost () / 2)
		{
			if (result->open (QIODevice::ReadOnly))
			{
				const auto& data = result->readAll ();
				CachePathContents_.insert (path, new QByteArray { data }, data.size ());
				if (!open)
					result->close ();
				else
					result->seek (0);
			}
		}

		if (open && !result->isOpen ())
			if (!result->open (QIODevice::ReadOnly))
				qWarning () << Q_FUNC_INFO
						<< "unable to open file"
						<< path
						<< result->errorString ();

		return result;
	}

	QIODevice_ptr ResourceLoader::Load (const QString& pathVariant, bool open) const
	{
		return Load (QStringList { pathVariant }, open);
	}

	QIODevice_ptr ResourceLoader::GetIconDevice (const QString& basename, bool open) const
	{
		return Load (IconizeBasename (basename), open);
	}

	QPixmap ResourceLoader::LoadPixmap (const QString& basename) const
	{
		if (CachePixmaps_.contains (basename))
			return *CachePixmaps_ [basename];

		auto dev = GetIconDevice (basename, true);
		if (!dev)
			return QPixmap ();

		const auto& data = dev->readAll ();

		QPixmap result;
		result.loadFromData (data);
		CachePixmaps_.insert (basename, new QPixmap (result), data.size ());
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

	void ResourceLoader::SetVerbose (bool verbose)
	{
		Verbose_ = verbose;
	}

	void ResourceLoader::ScanPath (const QString& path)
	{
		for (const auto& entry : QDir (path).entryList (NameFilters_, AttrFilters_))
		{
			Entry2Paths_ [entry] << path;
			if (!SubElemModel_->findItems (entry).isEmpty ())
				continue;

			SubElemModel_->appendRow (new QStandardItem (entry));
		}
	}

	void ResourceLoader::HandleDirectoryChanged (const QString& path)
	{
		emit watchedDirectoriesChanged ();

		for (auto& paths : Entry2Paths_)
			paths.removeAll (path);

		QFileInfo fi (path);
		if (fi.exists () &&
				fi.isDir () &&
				fi.isReadable ())
			ScanPath (path);

		for (auto i = Entry2Paths_.begin (); i != Entry2Paths_.end ();)
			if (i->isEmpty ())
			{
				for (auto item : SubElemModel_->findItems (i.key ()))
					SubElemModel_->removeRow (item->row ());
				i = Entry2Paths_.erase (i);
			}
			else
				++i;
	}
}
