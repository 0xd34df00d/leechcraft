/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

class QUrl;
class QIcon;

namespace LC::Poshuku::WebEngineView
{
	class IconDatabaseOnDisk
	{
	public:
		struct PageUrl2IconUrlRecord;
		struct IconUrl2IconRecord;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<IconUrl2IconRecord> IconUrl2Icon_;
		Util::oral::ObjectInfo_ptr<PageUrl2IconUrlRecord> PageUrl2IconUrl_;
	public:
		explicit IconDatabaseOnDisk ();
		~IconDatabaseOnDisk ();

		void UpdateIcon (const QUrl& pageUrl, const QIcon& icon, const QUrl& iconUrl);
		QIcon GetIcon (const QUrl& iconUrl);

		QList<std::tuple<QUrl, QUrl>> GetAllPages () const;
	};
}
