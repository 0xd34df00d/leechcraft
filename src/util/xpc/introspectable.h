/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QHash>
#include <QMetaType>
#include <QVariantMap>
#include "xpcconfig.h"

namespace LC
{
namespace Util
{
	using Introspect_f = std::function<QVariantMap (QVariant)>;

	class UTIL_XPC_API Introspectable
	{
		QHash<int, Introspect_f> Intros_;

		Introspectable () = default;
	public:
		Introspectable (const Introspectable&) = delete;
		Introspectable& operator= (const Introspectable&) = delete;

		static Introspectable& Instance ();

		template<typename T, typename U>
		void Register (const U& intro, std::result_of_t<U (QVariant)>* = nullptr)
		{
			const auto id = qMetaTypeId<T> ();
			Intros_ [id] = intro;
		}

		template<typename T, typename U>
		void Register (const U& intro, std::result_of_t<U (T)>* = nullptr)
		{
			Register<T> ([intro] (const QVariant& var) { return std::invoke (intro, var.value<T> ()); });
		}

		template<typename T>
		QVariantMap operator() (const T& t) const
		{
			return (*this) (QVariant::fromValue<T> (t));
		}

		QVariantMap operator() (const QVariant&) const;
	};
}
}
