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

#include "lyricscache.h"
#include <stdexcept>
#include <QFile>
#include <QtDebug>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			LyricsCache::LyricsCache ()
			{
				try
				{
					LeechCraft::Util::CreateIfNotExists ("deadlyrics/cache");
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}
			
				Dir_ = QDir::homePath ();
				Dir_.cd (".leechcraft/deadlyrics/cache");
			}
			
			LyricsCache& LyricsCache::Instance ()
			{
				static LyricsCache lc;
				return lc;
			}
			
			Lyrics LyricsCache::GetLyrics (const QByteArray& hash) const
			{
				if (Dir_.exists (hash.toHex ()))
				{
					QFile file (Dir_.filePath (hash.toHex ()));
					if (file.open (QIODevice::ReadOnly))
					{
						QByteArray raw = file.readAll ();
						QDataStream in (raw);
						Lyrics lyrics;
						in >> lyrics;
						return lyrics;
					}
					else
					{
						qWarning () << Q_FUNC_INFO
							<< "could not open (read) file"
							<< file.fileName ();
						throw std::runtime_error ("Could not open file");
					}
				}
				else
					throw std::runtime_error ("No such lyrics");
			}
			
			void LyricsCache::SetLyrics (const QByteArray& hash, const Lyrics& lyrics)
			{
				QFile file (Dir_.filePath (hash.toHex ()));
				if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
				{
					qWarning () << Q_FUNC_INFO
						<< "could not open (write|truncate) file"
						<< file.fileName ();
					return;
				}
				QByteArray data;
				QDataStream out (&data, QIODevice::WriteOnly);
				out << lyrics;
				file.write (data);
			}
		};
	};
};

