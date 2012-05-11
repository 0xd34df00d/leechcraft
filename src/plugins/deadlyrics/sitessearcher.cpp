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

#include "sitessearcher.h"
#include <QDomDocument>
#include <QFile>
#include <QtDebug>
#include "concretesite.h"

namespace LeechCraft
{
namespace DeadLyrics
{
	SitesSearcher::SitesSearcher (const QString& configPath, ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
		QFile file (configPath);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< configPath;
			return;
		}

		QDomDocument doc;
		if (!doc.setContent (&file))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse"
					<< configPath;
			return;
		}

		auto provider = doc.documentElement ().firstChildElement ("provider");
		while (!provider.isNull ())
		{
			try
			{
				Descs_ << ConcreteSiteDesc (provider);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error adding a provider:"
						<< e.what ();
			}
			provider = provider.nextSiblingElement ("provider");
		}
	}

	void SitesSearcher::Search (const Media::LyricsQuery& query, Media::QueryOptions)
	{
		Q_FOREACH (const auto& desc, Descs_)
			connect (new ConcreteSite (query, desc, Proxy_, this),
					SIGNAL (gotLyrics (Media::LyricsQuery, QStringList)),
					this,
					SIGNAL (gotLyrics (Media::LyricsQuery, QStringList)));
	}
}
}
