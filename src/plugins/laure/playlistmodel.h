/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LAURE_PLAYLISTMODEL_H
#define PLUGINS_LAURE_PLAYLISTMODEL_H
#include <QStandardItemModel>

namespace LeechCraft
{
namespace Laure
{
	/** @brief Provides a model for storing playlist data.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlayListModel : public QStandardItemModel
	{
		Q_OBJECT
		
		QStringList HeaderNames_;
	public:
		enum
		{
			/** @brief The data used in NowPlayingDelegate for determine
			 * the now playing item.
			 */
			IsPlayingRole = Qt::UserRole + 1
		};
		
		/** @brief Constructs a new PlayListModel class
		 * with the given parent.
		 */
		PlayListModel (QObject *);
		
		/** @brief Returns the item flags for the given index.
		 */
		Qt::ItemFlags flags (const QModelIndex&) const;
	};
}
}

#endif // PLUGINS_LAURE_PLAYLISTMODEL_H
