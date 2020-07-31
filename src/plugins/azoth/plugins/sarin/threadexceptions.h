/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFuture>
#include <util/threads/concurrentexception.h>
#include <tox/toxav.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	using ToxException = Util::ConcurrentStdException;

	class TextExceptionBase
	{
		const QByteArray Msg_;
	public:
		TextExceptionBase (const QString& = {});

		const char* what () const noexcept;
	};

	using ThreadException = Util::ConcurrentException<Util::NewType<TextExceptionBase, NewTypeTag>>;
	using UnknownFriendException = Util::ConcurrentException<Util::NewType<TextExceptionBase, NewTypeTag>>;

	template<typename CodeType = int>
	class TypedCodeExceptionBase
	{
		const CodeType Code_;
		const QByteArray Msg_;
	public:
		TypedCodeExceptionBase (CodeType code)
		: Code_ { code }
		, Msg_ { QString { "Unable to perform action: %1." }.arg (code).toUtf8 () }
		{
		}

		TypedCodeExceptionBase (const QByteArray& context, CodeType code)
		: Code_ { code }
		, Msg_ { "Unable to perform action " + context + ": " + QByteArray::number (code) }
		{
		}

		const char* what () const noexcept
		{
			return Msg_;
		}

		CodeType GetCode () const noexcept
		{
			return Code_;
		}
	};

	using CallInitiateException = Util::ConcurrentException<Util::NewType<TypedCodeExceptionBase<TOXAV_ERR_CALL>, NewTypeTag>>;
	using FrameSendException = Util::ConcurrentException<Util::NewType<TypedCodeExceptionBase<TOXAV_ERR_SEND_FRAME>, NewTypeTag>>;
	using CallAnswerException = Util::ConcurrentException<Util::NewType<TypedCodeExceptionBase<TOXAV_ERR_ANSWER>, NewTypeTag>>;

	template<typename T>
	using CommandCodeException = Util::ConcurrentException<TypedCodeExceptionBase<T>>;

	template<typename T>
	CommandCodeException<T> MakeCommandCodeException (const QByteArray& msg, T error)
	{
		return { msg, error };
	}
}
}
}
