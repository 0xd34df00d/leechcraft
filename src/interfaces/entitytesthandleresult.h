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

#ifndef INTERFACES_ENTITYTESTHANDLERESULT_H
#define INTERFACES_ENTITYTESTHANDLERESULT_H

/** @brief The result of testing whether the entity could be handled.
 * 
 * Both processing an Entity with IEntityHandler and IDownload are
 * considered to be "handling".
 */
struct EntityTestHandleResult
{
	/** @brief The priority with which an entity could be handled.
	 * 
	 * Typically the handler with the highest priority will be chosen.
	 * 
	 * A value of 0 or lower means that the given entity can't be
	 * handled by this handler/downloader at all.
	 */
	int HandlePriority_;
	
	enum Priority
	{
		PIdeal = 1000,
		PHigh = 800,
		PNormal = 600,
		PLow = 200,
		PNone = 0
	};
	
	/** @brief Whether other handlers should be canceled.
	 * 
	 * If this is set to true, then other handlers won't be called to
	 * handle the given entity.
	 */
	bool CancelOthers_;

	EntityTestHandleResult ()
	: HandlePriority_ ()
	, CancelOthers_ (false)
	{
	}
	
	explicit EntityTestHandleResult (Priority prio)
	: HandlePriority_ (prio)
	, CancelOthers_ (false)
	{
	}
};

#endif
