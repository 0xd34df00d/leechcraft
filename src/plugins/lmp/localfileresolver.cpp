/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localfileresolver.h"
#include <QtDebug>
#include <QFileInfo>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include "util/lmp/gstutil.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace LMP
{
	TagLib::FileRef LocalFileResolver::GetFileRef (const QString& file) const
	{
#ifdef Q_OS_WIN32
		return TagLib::FileRef (reinterpret_cast<const wchar_t*> (file.utf16 ()));
#else
		return TagLib::FileRef (file.toUtf8 ().constData (), true, TagLib::AudioProperties::Accurate);
#endif
	}

	LocalFileResolver::ResolveResult_t LocalFileResolver::ResolveInfo (const QString& file)
	{
		const auto& modified = QFileInfo (file).lastModified ();

		{
			QReadLocker locker (&CacheLock_);
			if (Cache_.contains (file))
			{
				const auto& pair = Cache_ [file];
				if (pair.first == modified)
					return ResolveResult_t::Right (pair.second);
			}
		}

		QMutexLocker tlLocker (&TaglibMutex_);

		auto r = GetFileRef (file);
		auto tag = r.tag ();
		if (!tag)
			return ResolveResult_t::Left ({ file, "cannot get audio tags" });

		auto audio = r.audioProperties ();

		auto ftl = [] (const TagLib::String& str) { return QString::fromUtf8 (str.toCString (true)); };

		const auto& genres = ftl (tag->genre ()).split ('/', Qt::SkipEmptyParts);

		MediaInfo info
		{
			file,
			ftl (tag->artist ()),
			ftl (tag->album ()),
			ftl (tag->title ()),
			Util::Map (genres, [] (const QString& genre) { return genre.trimmed (); }),
			audio ? audio->length () : 0,
			static_cast<qint32> (tag->year ()),
			static_cast<qint32> (tag->track ())
		};
		{
			QWriteLocker locker (&CacheLock_);
			if (Cache_.size () > 200)
				Cache_.clear ();
			Cache_ [file] = qMakePair (modified, info);
		}
		return ResolveResult_t::Right (info);
	}

	QMutex& LocalFileResolver::GetMutex ()
	{
		return TaglibMutex_;
	}

	void LocalFileResolver::flushCache ()
	{
		QWriteLocker locker { &CacheLock_ };
		Cache_.clear ();
	}
}
}
