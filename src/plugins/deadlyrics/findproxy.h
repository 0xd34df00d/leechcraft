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

#ifndef PLUGINS_DEADLYRICS_FINDPROXY_H
#define PLUGINS_DEADLYRICS_FINDPROXY_H
#include <vector>
#include <QAbstractItemModel>
#include <interfaces/ifinder.h>
#include "searcher.h"

namespace LeechCraft
{
	namespace Util
	{
		class SelectableBrowser;
	};

	namespace Plugins
	{
		namespace DeadLyrics
		{
			class FindProxy : public QAbstractItemModel
							, public IFindProxy
			{
				Q_OBJECT
				Q_INTERFACES (IFindProxy);

				LeechCraft::Request Request_;
				std::vector<QByteArray> Hashes_;
				lyrics_t Lyrics_;
				Util::SelectableBrowser *LyricsHolder_;
				QString ErrorString_;
				bool FetchedSomething_;

				FindProxy (const FindProxy&);
				FindProxy& operator= (const FindProxy&);
			public:
				FindProxy (const LeechCraft::Request&, QObject* = 0);
				virtual ~FindProxy ();

				QAbstractItemModel* GetModel ();

				int columnCount (const QModelIndex&) const;
				QVariant data (const QModelIndex&, int) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex&) const;
			private slots:
				void handleTextFetched (const LeechCraft::Plugins::DeadLyrics::Lyrics&, const QByteArray&);
				void handleError (const QString&);
			};
		};
	};
};

#endif

