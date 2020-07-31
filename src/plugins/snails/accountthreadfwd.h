/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <vmime/exception.hpp>
#include <vmime/security/cert/X509Certificate.hpp>
#include <vmime/security/cert/certificateException.hpp>
#include <util/sll/void.h>
#include <util/sll/eitherfwd.h>

namespace LC
{
namespace Snails
{
	class GenericExceptionWrapper;

	template<typename... Rest>
	using InvokeError_t = std::variant<
				vmime::exceptions::authentication_error,
				vmime::exceptions::connection_error,
				vmime::security::cert::certificateException,
				GenericExceptionWrapper,
				Rest...
			>;

	template<typename R, typename... Rest>
	using EitherInvokeError_t = Util::Either<InvokeError_t<Rest...>, R>;

	namespace detail
	{
		template<typename>
		struct AsInvokeError;

		template<typename... List>
		struct AsInvokeError<std::variant<List...>>
		{
			using Type = InvokeError_t<List...>;
		};
	}

	template<typename Errs>
	using AsInvokeError_t = typename detail::AsInvokeError<Errs>::Type;

	namespace detail
	{
		template<typename, typename ErrList, typename... Errs>
		struct AddErrors;

		template<typename Res>
		struct AddErrors<void, Res>
		{
			using Type = Res;
		};

		template<template<typename...> class ErrCont, typename... Existing, typename Head, typename... Rest>
		struct AddErrors<
				std::enable_if_t<(std::is_same_v<Head, Existing> || ...)>,
				ErrCont<Existing...>,
				Head,
				Rest...
			> : AddErrors<void, ErrCont<Existing...>, Rest...> {};

		template<template<typename...> class ErrCont, typename... Existing, typename Head, typename... Rest>
		struct AddErrors<
				std::enable_if_t<!(std::is_same_v<Head, Existing> || ...)>,
				ErrCont<Existing...>,
				Head,
				Rest...
			> : AddErrors<void, ErrCont<Head, Existing...>, Rest...> {};
	}

	template<typename ErrList, typename... Errs>
	using AddErrors_t = typename detail::AddErrors<void, ErrList, Errs...>::Type;

	class AccountThread;
}
}
