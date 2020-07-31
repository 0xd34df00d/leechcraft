/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>
#include <variant>
#include <optional>

namespace LC
{
namespace Azoth
{
namespace GPGExceptions
{
	/** @brief A general GPG error.
	 *
	 * An error has a context (i. e. what was trying to be performed), an
	 * error message and an optional error code, if applicable, or -1
	 * otherwise.
	 *
	 * There are some more specific classes deriving from this one for
	 * more specific error cases:
	 * - NullPubkey
	 * - Encryption
	 *
	 * This class hierarchy has been designed to be used either with usual
	 * C++ exceptions or with monadic error handling (see Util::Either).
	 * There is an algebraic sum type for the latter which can be
	 * pattern-matched using Util::Visit(), see AnyException_t.
	 */
	class General : public std::runtime_error
	{
		QString Context_;
		int Code_ = -1;
		QString Message_;
	public:
		/** @brief Constructs the error with the given \em context
		 * description.
		 *
		 * @param[in] context The context of the error.
		 */
		General (const QString& context)
		: std::runtime_error { context.toStdString () }
		, Context_ { context }
		{
		}

		/** @brief Constructs the error with the given \em context,
		 * \em code and error \em message.
		 *
		 * @param[in] context The context of the error.
		 * @param[in] code The error code.
		 * @param[in] message The human-readable error message.
		 */
		General (const QString& context, int code, const QString& message)
		: std::runtime_error
		{
			context.toStdString () + std::to_string (code) + ": " + message.toStdString ()
		}
		, Context_ { context }
		, Code_ { code }
		, Message_ { message }
		{
		}

		/** @brief Constructs the error with the given \em code and error
		 * \em message.
		 *
		 * @param[in] code The error code.
		 * @param[in] msg The human-readable error message.
		 */
		General (int code, const QString& msg)
		: General { "Azoth GPG error", code, msg }
		{
		}

		/** @brief Returns the context of the error.
		 *
		 * @return The context of the error.
		 */
		const QString& GetContext () const
		{
			return Context_;
		}

		/** @brief Returns the error code, if applicable.
		 *
		 * @return The error code, if applicable, or -1 otherwise.
		 */
		int GetCode () const
		{
			return Code_;
		}

		/** @brief Returns the human-readable error message.
		 *
		 * @return The error message.
		 */
		const QString& GetMessage () const
		{
			return Message_;
		}
	};

	/** @brief An error resulting from a null (or unset) public key.
	 */
	class NullPubkey : public General
	{
	public:
		/** @brief Constructs the error object.
		 */
		NullPubkey ()
		: General { "Azoth GPG: null pubkey" }
		{
		}
	};

	/** @brief Encryption failure.
	 */
	class Encryption : public General
	{
	public:
		/** @brief Constructs the error object using the error \em code
		 * and \em message.
		 *
		 * @param[in] code Error code as in the underlying library (like
		 * QCA or gpgme).
		 * @param[in] message The error message.
		 */
		Encryption (int code, const QString& message)
		: General { "Azoth GPG encryption error", code, message }
		{
		}
	};

	/** @brief A sum type of all the possible GPG-related errors.
	 */
	using AnyException_t = std::variant<Encryption, NullPubkey, General>;

	/** @brief A type representing a possibility of a GPG-related error.
	 */
	using MaybeException_t = std::optional<AnyException_t>;
}
}
}
