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

#ifndef PLUGINS_DEADLYRICS_CORE_H
#define PLUGINS_DEADLYRICS_CORE_H
#include <QAbstractItemModel>
#include "searcher.h"

class QNetworkAccessManager;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			class Core : public QObject
			{
				Q_OBJECT

				searchers_t Searchers_;
				QNetworkAccessManager *Manager_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();

				void SetNetworkAccessManager (QNetworkAccessManager*);
				QNetworkAccessManager* GetNetworkAccessManager () const;
				QStringList GetCategories () const;

				/** Returns all the searches for the given category. It's assumed
				 * that different calls to this function with the same category
				 * return the same searchers in the same order.
				 *
				 * @param[in] category The category for which one wants to get the
				 * searchers.
				 * @return The searchers for the passed category.
				 */
				searchers_t GetSearchers (const QString& category) const;
			};
		};
	};
};

#endif

