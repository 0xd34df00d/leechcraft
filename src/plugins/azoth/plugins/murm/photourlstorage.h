/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

class QUrl;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class PhotoUrlStorage : public QObject
	{
	public:
		struct Record;
	private:
		QSqlDatabase DB_;
		Util::oral::ObjectInfo_ptr<Record> AdaptedRecord_;
	public:
		PhotoUrlStorage (QObject* = nullptr);

		std::optional<QUrl> GetUserUrl (qulonglong);
		void SetUserUrl (qulonglong, const QUrl&);
	};
}
}
}
