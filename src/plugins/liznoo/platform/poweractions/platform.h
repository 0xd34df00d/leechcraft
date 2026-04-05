/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/sll/either.h>
#include <util/sll/void.h>
#include <util/threads/coro/taskfwd.h>

namespace LC::Liznoo::PowerActions
{
	class Platform : public QObject
	{
	public:
		using QObject::QObject;

		enum class State : std::uint8_t
		{
			Suspend,
			Hibernate
		};

		using Success = Util::Void;
		struct Fail { QString Reason_; };

		using Result = Util::Either<Fail, Success>;

		virtual Util::ContextTask<bool> IsAvailable () = 0;

		virtual Util::ContextTask<Result> CanChangeState (State) = 0;
		virtual Util::ContextTask<Result> ChangeState (State) = 0;
	};
}
