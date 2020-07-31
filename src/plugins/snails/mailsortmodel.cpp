/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mailsortmodel.h"
#include "mailmodel.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Snails
{
	MailSortModel::MailSortModel (QObject *parent)
	: QSortFilterProxyModel { parent }
	{
		XmlSettingsManager::Instance ().RegisterObject ({
					"RootsRespectRead",
					"RootsRespectReadChildren"
				},
				this, "handleRespectUnreadRootsChanged");
		handleRespectUnreadRootsChanged ();
	}

	bool MailSortModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		if (left.parent ().isValid () ||
				right.parent ().isValid () ||
				!RespectUnreadRoots_)
			return QSortFilterProxyModel::lessThan (left, right);

		auto leftRead = left.data (MailModel::MailRole::IsRead).toBool ();
		auto rightRead = right.data (MailModel::MailRole::IsRead).toBool ();

		if (RespectUnreadChildren_)
		{
			if (left.data (MailModel::MailRole::UnreadChildrenCount).toInt ())
				leftRead = false;
			if (right.data (MailModel::MailRole::UnreadChildrenCount).toInt ())
				rightRead = false;
		}

		if (leftRead == rightRead)
			return QSortFilterProxyModel::lessThan (left, right);

		return leftRead && !rightRead;
	}

	void MailSortModel::handleRespectUnreadRootsChanged ()
	{
		RespectUnreadRoots_ = XmlSettingsManager::Instance ()
				.property ("RootsRespectRead").toBool ();
		RespectUnreadChildren_ = XmlSettingsManager::Instance ()
				.property ("RootsRespectReadChildren").toBool ();
		invalidate ();
	}
}
}
