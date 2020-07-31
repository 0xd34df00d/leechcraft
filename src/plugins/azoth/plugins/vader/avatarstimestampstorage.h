/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>
#include <util/db/closingdb.h>

class QDateTime;

namespace LC
{
namespace Azoth
{
namespace Vader
{
	class AvatarsTimestampStorage
	{
	public:
		struct AvatarTimestamp;
	private:
		Util::ClosingDB DB_;

		Util::oral::ObjectInfo_ptr<AvatarTimestamp> Adapted_;
	public:
		AvatarsTimestampStorage ();

		std::optional<QDateTime> GetTimestamp (const QString&);
		void SetTimestamp (const QString&, const QDateTime&);
	};
}
}
}
