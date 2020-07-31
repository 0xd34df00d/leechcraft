/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>
#include <QObject>
#include <QHash>
#include <QReadWriteLock>
#include <QMutex>
#include <QDateTime>
#include <taglib/fileref.h>
#include "interfaces/lmp/itagresolver.h"
#include "mediainfo.h"

namespace LC
{
namespace LMP
{
	class LocalFileResolver : public QObject
							, public ITagResolver
	{
		Q_OBJECT
		Q_INTERFACES (LC::LMP::ITagResolver)

		QMutex TaglibMutex_;
		QReadWriteLock CacheLock_;
		QHash<QString, QPair<QDateTime, MediaInfo>> Cache_;
	public:
		using QObject::QObject;

		TagLib::FileRef GetFileRef (const QString&) const;
		ResolveResult_t ResolveInfo (const QString&);
		QMutex& GetMutex ();
	private slots:
		void flushCache ();
	};
}
}
