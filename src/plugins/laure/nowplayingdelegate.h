/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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

#pragma once
#include <QItemDelegate>

class QPainter;

namespace LeechCraft
{
namespace Laure
{
	class PlayListModel;
	
	/** @brief Provides display facilities for playlist items
	 * from a model.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class NowPlayingDelegate : public QItemDelegate
	{
		Q_OBJECT
	public:
		/** @brief Constructs a new NowPlayingDelegate class
		 * with the given parent.
		 */
		NowPlayingDelegate (QObject* = 0);
		
		/** @brief Renders the delegate using the given painter and
		 * style option for the item specified by index.
		 */
		void paint (QPainter *painter, const QStyleOptionViewItem& option,
				const QModelIndex& id) const;
	};
}
}