/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC::Util
{
	template<typename L, typename R>
	concept Concatable = requires (L&& l, R&& r) { std::forward<L> (l) + std::forward<R> (r); };

	class TaintedString
	{
		QString String_;
	public:
		template<typename... Args>
			requires std::constructible_from<QString, Args&&...>
		explicit TaintedString (Args&&... args)
		: String_ { std::forward<Args> (args)... }
		{
		}

		TaintedString (const TaintedString&) = default;
		TaintedString (TaintedString&&) = default;
		TaintedString& operator= (const TaintedString&) = default;
		TaintedString& operator= (TaintedString&&) = default;

		[[nodiscard]] QString ToHtmlEscaped () const
		{
			return String_.toHtmlEscaped ();
		}

		[[nodiscard]] QString UnsafeGetRaw () const
		{
			return String_;
		}

		bool IsEmpty () const
		{
			return String_.isEmpty ();
		}

		template<typename... Args>
			requires (std::is_same_v<TaintedString, std::decay_t<Args>>&& ...)
		friend TaintedString Format (const QString& pattern, Args&&... args)
		{
			return TaintedString { pattern.arg (std::forward_like<Args> (args.String_)...) };
		}

		friend TaintedString operator+ (const TaintedString& l, const TaintedString& r)
		{
			return TaintedString { l.String_ + r.String_ };
		}

		template<typename T>
			requires (!std::same_as<std::decay_t<T>, TaintedString>) && Concatable<const QString&, T&&>
		friend TaintedString operator+ (const TaintedString& l, T&& r)
		{
			return TaintedString { l.String_ + std::forward<T> (r) };
		}

		template<typename T>
			requires (!std::same_as<std::decay_t<T>, TaintedString>) && Concatable<T&&, const QString&>
		friend TaintedString operator+ (T&& l, const TaintedString& r)
		{
			return TaintedString { std::forward<T> (l) + r.String_ };
		}
	};
}
