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

#ifndef PLUGINS_AGGREGATOR_OPMLITEM_H
#define PLUGINS_AGGREGATOR_OPMLITEM_H
#include <QString>
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct OPMLItem
			{
				QString URL_;
				QString HTMLUrl_;
				QString Title_;
				QString Description_;
				QStringList Categories_;
				int MaxArticleAge_;
				int FetchInterval_;
				int MaxArticleNumber_;
				bool CustomFetchInterval_;
				//	<outline htmlUrl="" title="Оформление KDE"
				//	useCustomFetchInterval="false" maxArticleAge="0"
				//	fetchInterval="0" maxArticleNumber="0"
				//	archiveMode="globalDefault" version="RSS" type="rss"
				//	xmlUrl="http://www.kde.org/kde-look-content.rdf"
				//	id="2097705275" text="Оформление KDE" description="" />
			};
		};
	};
};

#endif

