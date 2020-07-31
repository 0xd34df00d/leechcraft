/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "m3u.h"
#include <memory>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC
{
namespace LMP
{
namespace M3U
{
	namespace
	{
		QPair<QString, QVariant> ParseMetadata (QString str)
		{
			const auto eqIdx = str.indexOf ('=');
			if (eqIdx == -1)
				return {};

			return { str.mid (1, eqIdx - 1), str.mid (eqIdx + 1) };
		}
	}

	Playlist Read2Sources (const QString& path)
	{
		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< path
					<< file.errorString ();
			return {};
		}

		const auto& m3uDir = QFileInfo (path).absoluteDir ();

		QVariantMap lastMetadata;

		Playlist result;
		while (!file.atEnd ())
		{
			const auto& line = file.readLine ().trimmed ();
			if (line.startsWith ('#'))
			{
				const auto& pair = ParseMetadata (line);
				if (!pair.first.isEmpty ())
					lastMetadata [pair.first] = pair.second;
				continue;
			}

			const auto& url = QUrl::fromEncoded (line);
			auto src = QString::fromUtf8 (line);

			const auto mdGuard = std::shared_ptr<void> (nullptr,
					[&lastMetadata] (void*) { lastMetadata.clear (); });

#ifdef Q_OS_WIN32
			if (url.scheme ().size () > 1)
#else
			if (!url.scheme ().isEmpty ())
#endif
			{
				result.Append ({ url, lastMetadata });
				continue;
			}

			src.replace ('\\', '/');

			const QFileInfo fi (src);
			if (fi.isRelative ())
				src = m3uDir.absoluteFilePath (src);

			if (fi.suffix () == "m3u" || fi.suffix () == "m3u8")
				result += Read2Sources (src);
			else
				result.Append ({ src, lastMetadata });
		}
		return result;
	}

	void Write (const QString& path, const Playlist& sources)
	{
		QFile file (path);
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< path
					<< file.errorString ();
			return;
		}

		for (const auto& item : sources)
		{
			for (const auto& pair : Util::Stlize (item.Additional_))
				file.write (("#" + pair.first + "=" + pair.second.toString () + "\n").toUtf8 ());
			file.write (item.Source_.ToUrl ().toEncoded ());
			file.write ("\n");
		}
	}
}
}
}
