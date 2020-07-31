/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linkhistory.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	void LinkHistory::addHistoryEntry (const QString& url)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("StoreLocalLinkHistory").toBool ())
			return;

		History_ << url;
	}

	bool LinkHistory::historyContains (const QString& url) const
	{
		if (!XmlSettingsManager::Instance ()
				.property ("StoreLocalLinkHistory").toBool ())
			return false;

		return History_.contains (url);
	}
}
}
}
