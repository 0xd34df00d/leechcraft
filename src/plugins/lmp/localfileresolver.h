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

namespace LeechCraft
{
namespace LMP
{
	class ResolveError : public std::runtime_error
	{
		QString Path_;
	public:
		ResolveError (const QString&, const std::string&);
		~ResolveError () throw ();

		QString GetPath () const;
	};

	class LocalFileResolver : public QObject
							, public ITagResolver
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::LMP::ITagResolver)

		QMutex TaglibMutex_;
		QReadWriteLock CacheLock_;
		QHash<QString, QPair<QDateTime, MediaInfo>> Cache_;
	public:
		LocalFileResolver (QObject* = 0);

		TagLib::FileRef GetFileRef (const QString&) const;
		MediaInfo ResolveInfo (const QString&);
		QMutex& GetMutex ();
	};
}
}
