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

#include "localfileresolver.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>

namespace LeechCraft
{
namespace LMP
{
	LocalFileResolver::LocalFileResolver (QObject *parent)
	: QObject (parent)
	{
	}

	MediaInfo LocalFileResolver::ResolveInfo (const QString& file)
	{
		{
			QReadLocker locker (&CacheLock_);
			if (Cache_.contains (file))
				return Cache_ [file];
		}

		TagLib::FileRef r (file.toUtf8 ().constData ());
		auto tag = r.tag ();
		auto audio = r.audioProperties ();

		auto ftl = [] (const TagLib::String& str) { return QString::fromUtf8 (str.toCString (true)); };

		auto genres = ftl (tag->genre ()).split ('/', QString::SkipEmptyParts);
		std::for_each (genres.begin (), genres.end (),
				[] (QString& genre) { genre = genre.trimmed (); });

		MediaInfo info =
		{
			file,
			ftl (tag->artist ()),
			ftl (tag->album ()),
			ftl (tag->title ()),
			genres,
			audio ? audio->length () : 0,
			tag->year (),
			tag->track ()
		};
		{
			QWriteLocker locker (&CacheLock_);
			if (Cache_.size () > 200)
				Cache_.clear ();
			Cache_ [file] = info;
		}
		return info;
	}
}
}
