/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filesource.h"
#include <QFile>
#include <QtDebug>
#include <interfaces/media/audiostructs.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Xtazy
{
	FileSource::FileSource (QObject *parent)
	: TuneSourceBase { "File", parent }
	{
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
			EmitChange ({});
			return;
		}

		const QString& data = QString::fromUtf8 (file.readAll ());
		if (data.isEmpty ())
		{
			EmitChange ({});
			return;
		}

		QVariantMap result;
		for (auto line : data.splitRef ('\n', Qt::SkipEmptyParts))
		{
			line = line.trimmed ();
			const int idx = line.indexOf (' ');
			if (idx == -1)
				continue;

			const auto& key = line.left (idx);
			const auto& val = line.mid (idx + 1);
			result [key.toString ().toLower ()] = val.toString ();
		}

		EmitChange (FromMPRISMap (result));
	}

	void FileSource::handleFilePathChanged ()
	{
		const auto& watched = Watcher_.files ();
		if (!watched.isEmpty ())
			Watcher_.removePaths (watched);

		const QString& path = XmlSettingsManager::Instance ().property ("FileSourcePath").toString ();
		if (path.isEmpty ())
			return;

		Watcher_.addPath (path);
		handleFileChanged (path);
	}
}
}
