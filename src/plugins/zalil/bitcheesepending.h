/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "pendinguploadbase.h"

namespace LC
{
namespace Zalil
{
	class BitcheesePending final : public PendingUploadBase
	{
	public:
		BitcheesePending (const QString&, const ICoreProxy_ptr&, QObject* = nullptr);
	private:
		void handleFinished () override;
	};
}
}
