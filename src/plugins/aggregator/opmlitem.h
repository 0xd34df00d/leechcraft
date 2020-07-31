/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QStringList>

namespace LC
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
}
}
