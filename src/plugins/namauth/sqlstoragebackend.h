/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include <QObject>
#include <util/db/oral/oralfwd.h>
#include <util/db/oral/oraltypes.h>
#include <util/sll/ctstring.h>

namespace LC
{
namespace NamAuth
{
	class SQLStorageBackend : public QObject
	{
		Q_OBJECT

		std::shared_ptr<QSqlDatabase> DB_;
	public:
		struct AuthRecord
		{
			QString RealmName_;
			QString Context_;
			QString Login_;
			QString Password_;

			constexpr static auto ClassName = "AuthRecords"_ct;

			template<Util::CtString Str>
			constexpr static auto FieldNameMorpher ()
			{
				return Str;
			}

			using Constraints = Util::oral::Constraints<
					Util::oral::PrimaryKey<0, 1>
				>;
		};
	private:
		Util::oral::ObjectInfo_ptr<AuthRecord> AdaptedRecord_;
	public:
		SQLStorageBackend ();

		static QString GetDBPath ();

		std::optional<AuthRecord> GetAuth (const QString&, const QString&);
		void SetAuth (const AuthRecord&);
	};
}
}
