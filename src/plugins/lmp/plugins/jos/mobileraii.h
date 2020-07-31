/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>
#include <typeinfo>
#include <QString>
#include <libimobiledevice/libimobiledevice.h>

namespace LC
{
namespace LMP
{
namespace jOS
{
	class MobileRaiiException : public std::runtime_error
	{
		uint16_t ErrCode_;
	public:
		MobileRaiiException (const std::string&, uint16_t);
		~MobileRaiiException () noexcept;

		uint16_t GetErrCode () const;
	};

	template<typename T, typename DeleterRes = int16_t, typename Deleter = DeleterRes (*) (T)>
	class MobileRaii
	{
		T Type_ = nullptr;
		const Deleter Deleter_;
	public:
		typedef MobileRaii<T, Deleter> type;

		template<typename Creator>
		MobileRaii (Creator c, Deleter d)
		: Deleter_ { d }
		{
			if (const auto ret = c (&Type_))
			{
				const auto& errStr = "Cannot create something: " + QString::number (ret) + " for " + typeid (T).name ();
				throw MobileRaiiException (errStr.toUtf8 ().constData (), ret);
			}
		}

		~MobileRaii ()
		{
			if (Type_)
				Deleter_ (Type_);
		}

		MobileRaii (MobileRaii&& other)
		: Type_ { other.Type_ }
		, Deleter_ { other.Deleter_ }
		{
			other.Type_ = nullptr;
		}

		MobileRaii (const MobileRaii&) = delete;
		MobileRaii& operator= (const MobileRaii&) = delete;

		operator T () const
		{
			return Type_;
		}
	};

	template<typename T, typename Creator, typename Deleter>
	MobileRaii<T, std::result_of_t<Deleter (T)>, Deleter> MakeRaii (Creator c, Deleter d)
	{
		return { c, d };
	}
}
}
}
