/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "azothutilconfig.h"

namespace LC::Azoth
{
	class ICLEntry;
	struct OutgoingMessage;

	class AZOTH_UTIL_API Hooks : public QObject
	{
	protected:
		using QObject::QObject;
	signals:
		void messageWillBeCreated (bool& cancel,
				ICLEntry& entry,
				OutgoingMessage& message);
	};
}
