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
#include "interfaces/azoth/ihaveavatars.h"

namespace LC
{
namespace Azoth
{
	class AvatarsStorageOnDisk : public QObject
	{
	public:
		struct Record;
	private:
		QSqlDatabase DB_;
		Util::oral::ObjectInfo_ptr<Record> AdaptedRecord_;
	public:
		AvatarsStorageOnDisk (QObject* = nullptr);
		~AvatarsStorageOnDisk ();

		void SetAvatar (const QString& entryId, IHaveAvatars::Size size, const QByteArray& imageData) const;
		std::optional<QByteArray> GetAvatar (const QString& entryId, IHaveAvatars::Size size) const;
		void DeleteAvatars (const QString& entryId) const;
	};
}
}
