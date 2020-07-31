/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>
#include <QDataStream>
#include <QByteArray>

namespace LC::Util::oral
{
	template<typename T>
	struct AsDataStream
	{
		using BaseType = QByteArray;

		T Val_;

		operator T () const &
		{
			return Val_;
		}

		operator T&& () &&
		{
			return std::move (Val_);
		}

		AsDataStream () = default;
		AsDataStream (const AsDataStream&) = default;
		AsDataStream (AsDataStream&&) = default;

		AsDataStream& operator= (const AsDataStream&) = default;
		AsDataStream& operator= (AsDataStream&&) = default;

		template<typename... Args>
		AsDataStream (Args&&... args)
		: Val_ { std::forward<Args> (args)... }
		{
		}

		template<typename U>
		AsDataStream& operator= (U&& val)
		{
			Val_ = std::forward<U> (val);
			return *this;
		}

		BaseType ToBaseType () const
		{
			QByteArray ba;
			{
				QDataStream out { &ba, QIODevice::WriteOnly };
				out << Val_;
			}
			return ba;
		}

		static AsDataStream FromBaseType (const QByteArray& ba)
		{
			QDataStream in { ba };

			AsDataStream res;
			in >> res.Val_;
			return res;
		}
	};

}
