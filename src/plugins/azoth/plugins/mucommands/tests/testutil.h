/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>
#include <QString>
#include <QtDebug>
#include <boost/variant.hpp>

namespace LC::Azoth::MuCommands
{
	template<typename T>
	constexpr bool TestDebuggable (decltype (std::declval<QDebug> () << T {})*)
	{
		return true;
	}

	template<typename T>
	constexpr bool TestDebuggable (...)
	{
		return false;
	}

	struct PrintVisitor
	{
		QString operator() (const std::string& str) const
		{
			return QString::fromStdString (str);
		}

		template<typename T>
		QString operator() (const T& t) const
		{
			QString result;
			if constexpr (TestDebuggable<T> (0))
				QDebug { &result } << t;
			else
				QDebug { &result } << typeid (t).name ();
			return result;
		}

		template<typename T1, typename T2>
		QString operator() (const std::pair<T1, T2>& t) const
		{
			QString result;
			QDebug { &result } << "std::pair { " << (*this) (t.first) << "; " << (*this) (t.second) << " }";
			return result;
		}

		template<typename... Args>
		QString operator() (const boost::variant<Args...>& variant) const
		{
			auto result = QString { "Variant with type %1, value: { `%2` }" }
					.arg (variant.which ())
					.arg (boost::apply_visitor (PrintVisitor {}, variant));
			return result;
		}
	};

	template<typename... Args>
	char* PrintVar (const boost::variant<Args...>& variant)
	{
		const auto& result = PrintVisitor {} (variant);
		return qstrdup (result.toUtf8 ().constData ());
	}
}
