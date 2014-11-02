/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFuture>
#include <util/sll/concurrentexception.h>
#include <util/sll/newtype.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Sarin
{
	using ToxException = Util::ConcurrentException<Util::ExceptionWrapperBase<std::exception>>;

	class TextExceptionBase
	{
		const QByteArray Msg_;
	public:
		TextExceptionBase (const QString& = {});

		const char* what () const noexcept;
	};

	using ThreadException = Util::ConcurrentException<Util::NewType<TextExceptionBase, NewTypeTag>>;
	using UnknownFriendException = Util::ConcurrentException<Util::NewType<TextExceptionBase, NewTypeTag>>;

	class CodeExceptionBase
	{
		int Code_;

		const QByteArray Msg_;
	public:
		CodeExceptionBase (int code);

		const char* what () const noexcept;

		int GetCode () const noexcept;
	};

	using CallInitiateCode = Util::NewType<CodeExceptionBase, NewTypeTag>;
	using FramePrepareCode = Util::NewType<CodeExceptionBase, NewTypeTag>;
	using FrameSendCode = Util::NewType<CodeExceptionBase, NewTypeTag>;

	using CallInitiateException = Util::ConcurrentException<CallInitiateCode>;
	using FramePrepareException = Util::ConcurrentException<FramePrepareCode>;
	using FrameSendException = Util::ConcurrentException<FrameSendCode>;
	using CallAnswerException = Util::ConcurrentException<Util::NewType<CodeExceptionBase, NewTypeTag>>;
}
}
}
