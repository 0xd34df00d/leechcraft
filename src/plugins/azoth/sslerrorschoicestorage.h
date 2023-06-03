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
#include <QSslError>
#include <util/db/oral/oralfwd.h>

namespace LC
{
namespace Azoth
{
	class SslErrorsChoiceStorage
	{
	public:
		struct Record;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<Record> AdaptedRecord_;
	public:
		SslErrorsChoiceStorage ();
		~SslErrorsChoiceStorage ();

		enum class Action
		{
			Ignore,
			Abort
		};

		std::optional<Action> GetAction (const QByteArray&, QSslError::SslError) const;
		void SetAction (const QByteArray&, QSslError::SslError, Action);
	};
}
}
