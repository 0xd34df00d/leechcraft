/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <exception>
#include <optional>
#include <QDir>
#include <QString>
#include <util/sll/eitherfwd.h>
#include <util/sll/void.h>

namespace LC
{
enum class ContentType;

namespace Snails
{
	enum class MsgType;
	class Account;

	class TemplatesStorage
	{
		const QDir RootDir_;
	public:
		TemplatesStorage ();

		using LoadError_t = std::runtime_error;
		using SaveError_t = std::runtime_error;

		using LoadResult_t = Util::Either<LoadError_t, std::optional<QString>>;
		using SaveResult_t = Util::Either<SaveError_t, Util::Void>;

		LoadResult_t LoadTemplate (ContentType, MsgType, const Account*);
		SaveResult_t SaveTemplate (ContentType, MsgType, const Account*, const QString&);
	};
}
}
