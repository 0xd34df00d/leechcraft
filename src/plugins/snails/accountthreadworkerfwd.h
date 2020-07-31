/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>
#include <memory>
#include <vmime/net/message.hpp>
#include <util/sll/either.h>
#include <util/sll/void.h>
#include "attdescr.h"
#include "messageinfo.h"

namespace LC
{
namespace Snails
{
	struct FolderNotFound
	{
		const char* what () const;
	};

	struct MessageNotFound
	{
		const char* what () const;
	};

	struct FileOpenError
	{
		const char* what () const;
	};

	struct AttachmentNotFound
	{
		const char* what () const;
	};

	struct MessageBodies;

	using FetchWholeMessageResult_t = Util::Either<std::variant<FolderNotFound, MessageNotFound>, MessageBodies>;

	using PrefetchWholeMessagesResult_t = Util::Either<std::variant<FolderNotFound>, QHash<QByteArray, MessageBodies>>;

	using FetchAttachmentResult_t = Util::Either<
			std::variant<MessageNotFound, FileOpenError, AttachmentNotFound>,
			Util::Void
		>;

	struct FetchedMessageInfo
	{
		MessageInfo Info_;
		vmime::shared_ptr<const vmime::header> Headers_;
	};

	struct FolderAlreadyExists
	{
		const char* what () const;
	};

	struct InvalidPathComponent
	{
		QString Component_;
		QByteArray FullMessage_ = "`" + Component_.toUtf8 () + "` is an invalid component name";

		const char* what () const;
	};

	using CreateFolderResult_t = Util::Either<std::variant<FolderAlreadyExists, InvalidPathComponent>, Util::Void>;
}
}
