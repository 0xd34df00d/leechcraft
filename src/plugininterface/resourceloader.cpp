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
#include <QtDebug>

namespace LeechCraft
{
	namespace Util
	{
		ResourceLoader::ResourceLoader (const QString& relPath, QObject* parent)
		: QObject (parent)
		, RelativePath_ (relPath)
		{
			if (RelativePath_.startsWith ('/'))
				RelativePath_ = RelativePath_.mid (1);
			if (!RelativePath_.endsWith ('/'))
				RelativePath_.append ('/');
		}

		void ResourceLoader::AddLocalPrefix (QString prefix)
		{
			if (!prefix.isEmpty () &&
					!prefix.endsWith ('/'))
				prefix.append ('/');
			LocalPrefixesChain_ << (QDir::homePath () + "/.leechcraft/data/" + prefix);
		}

		void ResourceLoader::AddGlobalPrefix (QString prefix)
		{
			if (!prefix.endsWith ('/'))
				prefix.append ('/');
			GlobalPrefixesChain_ << prefix;
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
	}
}
