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

	struct CharEnumTypeName
	{
		constexpr auto operator() () const noexcept
		{
			return "VARCHAR(1)"_ct;
		}
	};

	template<typename Fst, typename Snd>
	struct NttpPair
	{
		Fst first;
		Snd second;
	};

	template<typename Enum, NttpPair<Enum, char>... Chars>
		requires std::is_enum_v<Enum>
	struct ConvertEnum
	{
		static_assert (sizeof... (Chars) > 0);

		QVariant operator() (Enum val) const
		{
			for (const auto& [e, ch] : { Chars... })
				if (e == val)
					return QString { QChar { ch } };

			throw std::runtime_error { "invalid enum value " + std::to_string (static_cast<int> (val)) };
		}

		Enum operator() (const QVariant& var) const
		{
			if (const auto& str = var.toString ();
				str.size () == 1)
			{
				const auto ch = str.at (0).toLatin1 ();
				for (const auto& [k, v] : { Chars... })
					if (v == ch)
						return k;
			}

			qWarning () << "invalid stored value:" << var;
			return Chars... [0].first;
		}
	};
}
