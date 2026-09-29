/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <list>
#include <stdexcept>
#include <vector>
#include <QStringList>
#include "../threadsconfig.h"

class QObject;

namespace LC::Util
{
	class UTIL_THREADS_API ContextDeadException : public std::runtime_error
	{
	public:
		explicit ContextDeadException (const QString&);
	};

	struct ContextExtensionBase;

	class UTIL_THREADS_API CoroContext
	{
		friend struct AddContext;
		friend struct ContextExtensionBase;

	public:
		struct Coro
		{
			std::coroutine_handle<> Handle_;
			ContextExtensionBase *Promise_;
		};
		using Coros = std::list<Coro>;
	private:
		std::list<Coro> Coros_;
		QString Name_;
	public:
		struct Registration
		{
			CoroContext *Ctx_;
			Coros::iterator Pos_;
		};
		using Registrations = std::vector<Registration>;

		CoroContext () = default;
		explicit CoroContext (const QString&);
		explicit CoroContext (const QObject&);

		CoroContext (const CoroContext&) = delete;
		CoroContext (CoroContext&&) = delete;
		CoroContext& operator= (const CoroContext&) = delete;
		CoroContext& operator= (CoroContext&&) = delete;

		~CoroContext ();

		QString GetName () const { return Name_; }

		/** @brief Returns the context tied to the lifetime of `obj`.
		 *
		 * The same object always yields the same context, which dies when
		 * `obj` gets `QObject::destroyed()`.
		 *
		 * This function shall be called from the same thread where `obj`
		 * lives.
		 *
		 * @param[in] obj An object to whose lifetime the context is tied.
		 * @return The context representing the lifetime of `obj`.
		 */
		static CoroContext& Of (QObject& obj);
	};
}

/** @brief Declares the coroutine context of the enclosing QObject-derived class.
 *
 * Shall be the last thing in the class definition: the context is then
 * destroyed first, cancelling the coroutines while the other members are
 * still alive.
 */
#define LC_CORO_CONTEXT \
	private: \
		::LC::Util::CoroContext CoroContext_ { *this };
