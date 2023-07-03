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
#include <variant>
#include <QUrl>
#include <QtPlugin>
#include <util/sll/either.h>

namespace LC::Poshuku
{
	class IInternalSchemeHandler
	{
	protected:
		virtual ~IInternalSchemeHandler () = default;
	public:
		struct Request
		{
			QUrl Url_;
			QUrl Initiator_;
		};

		using ReplyContents = std::variant<QByteArray, std::shared_ptr<QIODevice>>;

		struct Reply
		{
			QByteArray ContentType_;
			ReplyContents Contents_;
		};

		enum class Error
		{
			Unsupported,

			NotFound,
			Denied,
		};

		using HandleResult = Util::Either<Error, Reply>;

		virtual HandleResult HandleRequest (const Request&) = 0;
	};
}

Q_DECLARE_INTERFACE (LC::Poshuku::IInternalSchemeHandler,
		"org.LeechCraft.Poshuku.IInternalSchemeHandler/1.0")
