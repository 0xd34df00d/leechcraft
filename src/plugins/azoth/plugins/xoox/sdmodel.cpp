/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sdmodel.h"
#include <QtDebug>
#include "sdsession.h"

namespace LC
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
