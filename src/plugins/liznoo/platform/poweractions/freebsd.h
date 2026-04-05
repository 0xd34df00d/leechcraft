/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include "platform.h"

namespace LC::Liznoo::PowerActions
{
	class FreeBSD final : public Platform
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Liznoo::PowerActions::FreeBSD)
	public:
		using Platform::Platform;

		Util::ContextTask<bool> IsAvailable () override;
		Util::ContextTask<Result> CanChangeState (State) override;
		Util::ContextTask<Result> ChangeState (State) override;
	};
}
