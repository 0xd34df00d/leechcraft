/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "sdmodel.h"
#include <QtDebug>
#include "sdsession.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	SDModel::SDModel (SDSession *session)
	: QStandardItemModel (session)
	, Session_ (session)
	{
	}
	
	bool SDModel::canFetchMore (const QModelIndex&) const
	{
		return true;
	}
	
	void SDModel::fetchMore (const QModelIndex& parent)
	{
		if (!parent.isValid () ||
				parent.data (SDSession::DRFetchedMore).toBool ())
			return;

		Session_->QueryItem (itemFromIndex (parent.sibling (parent.row (), 0)));
	}
	
	bool SDModel::hasChildren (const QModelIndex& parent) const
	{
		if (!parent.isValid ())
			return true;
		
		return parent.data (SDSession::DRFetchedMore).toBool () ?
				QStandardItemModel::hasChildren (parent) :
				true;
	}
}
}
}
