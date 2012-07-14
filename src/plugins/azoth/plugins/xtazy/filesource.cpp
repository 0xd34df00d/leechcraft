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

#include "filesource.h"
#include <QFile>
#include <QtDebug>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	FileSource::FileSource (QObject *parent)
	: TuneSourceBase (parent)
	{
		setObjectName ("FileSource");
		connect (&Watcher_,
				SIGNAL (fileChanged (const QString&)),
				this,
				SLOT (handleFileChanged (const QString&)),
				Qt::QueuedConnection);

		XmlSettingsManager::Instance ().RegisterObject ("FileSourcePath",
				this, "handleFilePathChanged");
		handleFilePathChanged ();
	}
	
	void FileSource::handleFileChanged (const QString& filePath)
	{
		QFile file (filePath);
		if (!file.exists () ||
				!file.open (QIODevice::ReadOnly))
		{
			emit tuneInfoChanged (TuneInfo_t ());
			return;
		}

		const QString& data = QString::fromUtf8 (file.readAll ());
		if (data.isEmpty ())
		{
			emit tuneInfoChanged (TuneInfo_t ());
			return;
		}
		
		TuneInfo_t result;
		Q_FOREACH (QString line, data.split ('\n', QString::SkipEmptyParts))
		{
			line = line.trimmed ();
			const int idx = line.indexOf (' ');
			if (idx == -1)
				continue;

			const QString& key = line.left (idx);
			const QString& val = line.mid (idx + 1);
			result [key.toLower ()] = val;
		}

		emit tuneInfoChanged (result);
	}
	
	void FileSource::handleFilePathChanged ()
	{
		const QStringList& watched = Watcher_.files ();
		if (!watched.isEmpty ())
			Watcher_.removePaths (watched);

		const QString& path = XmlSettingsManager::Instance ()
				.property ("FileSourcePath").toString ();
		if (path.isEmpty ())
			return;
		
		Watcher_.addPath (path);
		handleFileChanged (path);
	}
}
}
}
