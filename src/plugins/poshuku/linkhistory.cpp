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

#include "linkhistory.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			LinkHistory::LinkHistory (QObject *parent)
			: QWebHistoryInterface (parent)
			{
			}
			
			void LinkHistory::addHistoryEntry (const QString& url)
			{
				if (!History_.contains (url))
					History_ << url;
			}
			
			bool LinkHistory::historyContains (const QString& url) const
			{
				if (History_.contains (url))
					return true;
				return false;
			}
		};
	};
};

