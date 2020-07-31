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
#include <QUuid>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

namespace LC
{
	class TagsStorage : public QObject
	{
	public:
		struct Record;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<Record> Record_;
	public:
		explicit TagsStorage (QObject* = nullptr);

		using Id = QUuid;

		void AddTag (const Id&, const QString&);
		void DeleteTag (const Id&);
		void SetTagName (const Id&, const QString&);
		QList<QPair<Id, QString>> GetAllTags () const;
	};
}
