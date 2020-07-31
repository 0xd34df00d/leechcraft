/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "platform.h"

namespace LC
{
namespace Liznoo
{
namespace PowerActions
{
	class UPower final : public Platform
	{
	public:
		using Platform::Platform;

		QFuture<bool> IsAvailable () override;
		QFuture<QueryChangeStateResult> CanChangeState (State) override;
		void ChangeState (State) override;
	};
}
}
}
