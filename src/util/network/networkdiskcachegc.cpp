/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkdiskcachegc.h"
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QtConcurrentRun>
#include <QDateTime>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/sll/util.h>
#include <util/threads/futures.h>

namespace LC
{
namespace Util
{
	NetworkDiskCacheGC::NetworkDiskCacheGC ()
	{
		const auto timer = new QTimer { this };
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (handleCollect ()));
		timer->start (60 * 60 * 1000);
	}

	NetworkDiskCacheGC& NetworkDiskCacheGC::Instance ()
	{
		static NetworkDiskCacheGC gc;
		return gc;
	}

	namespace
	{
		struct SizeCollectInfo
		{
			QMultiMap<QDateTime, QString> Items_;
			qint64 TotalSize_ = 0;
		};

		SizeCollectInfo CollectSizes (const QString& cacheDirectory)
		{
			SizeCollectInfo result;

			const QDir::Filters filters = QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot;
			QDirIterator it { cacheDirectory, filters, QDirIterator::Subdirectories };

			while (it.hasNext ())
			{
				const auto& path = it.next ();
				const auto& info = it.fileInfo ();
				result.Items_.insert (info.birthTime (), path);
				result.TotalSize_ += info.size ();
			}

			return result;
		}
	}

	QFuture<qint64> NetworkDiskCacheGC::GetCurrentSize (const QString& path) const
	{
		return QtConcurrent::run ([path] { return CollectSizes (path).TotalSize_; });
	}

	Util::DefaultScopeGuard NetworkDiskCacheGC::RegisterDirectory (const QString& path,
			const std::function<int ()>& sizeGetter)
	{
		auto& list = Directories_ [path];
		list.push_front (sizeGetter);
		const auto thisItem = list.begin ();

		return Util::MakeScopeGuard ([this, path, thisItem] { UnregisterDirectory (path, thisItem); }).EraseType ();
	}

	void NetworkDiskCacheGC::UnregisterDirectory (const QString& path, CacheSizeGetters_t::iterator pos)
	{
		if (!Directories_.contains (path))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown directory"
					<< path;
			return;
		}

		auto& list = Directories_ [path];
		list.erase (pos);

		if (!list.isEmpty ())
			return;

		Directories_.remove (path);
		LastSizes_.remove (path);
	}

	namespace
	{
		qint64 Collector (const QString& cacheDirectory, qint64 goal)
		{
			if (cacheDirectory.isEmpty ())
				return 0;

			qDebug () << Q_FUNC_INFO << "running..." << cacheDirectory << goal;

			auto sizeInfoResult = CollectSizes (cacheDirectory);

			for (auto i = sizeInfoResult.Items_.constBegin ();
					i != sizeInfoResult.Items_.constEnd () && sizeInfoResult.TotalSize_ > goal;
					++i)
			{
				QFile file { *i };
				sizeInfoResult.TotalSize_ -= file.size ();
				file.remove ();
			}

			qDebug () << "collector finished" << sizeInfoResult.TotalSize_;

			return sizeInfoResult.TotalSize_;
		}
	};

	void NetworkDiskCacheGC::handleCollect ()
	{
		if (IsCollecting_)
		{
			qWarning () << Q_FUNC_INFO
					<< "already collecting";
			return;
		}

		QList<QPair<QString, int>> dirs;
		for (const auto& pair : Util::Stlize (Directories_))
		{
			const auto& getters = pair.second;
			const auto minSize = (*std::min_element (getters.begin (), getters.end (),
						Util::ComparingBy (Apply))) ();
			dirs.append ({ pair.first, minSize });
		}

		if (dirs.isEmpty ())
			return;

		IsCollecting_ = true;

		Util::Sequence (this,
				QtConcurrent::run ([dirs]
						{
							QMap<QString, qint64> sizes;
							for (const auto& pair : dirs)
								sizes [pair.first] = Collector (pair.first, pair.second);
							return sizes;
						})) >>
				[this] (const QMap<QString, qint64>& sizes)
				{
					IsCollecting_ = false;
					for (const auto& pair : Util::Stlize (sizes))
						LastSizes_ [pair.first] = pair.second;
				};
	}
}
}
