/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "xpcconfig.h"

class IEntityManager;

namespace LC
{
namespace Util
{
	class UTIL_XPC_API ScreensaverProhibitor final
	{
		bool Enabled_ = true;
		bool Prohibited_ = false;

		IEntityManager * const IEM_;
		const QString ContextID_;
	public:
		ScreensaverProhibitor (IEntityManager*);
		~ScreensaverProhibitor ();

		ScreensaverProhibitor (const ScreensaverProhibitor&) = delete;
		ScreensaverProhibitor (ScreensaverProhibitor&&) = delete;
		ScreensaverProhibitor& operator= (const ScreensaverProhibitor&) = delete;
		ScreensaverProhibitor& operator= (ScreensaverProhibitor&&) = delete;

		void SetProhibited (bool);
		void SetProhibitionsEnabled (bool);
	private:
		void SendEntity (bool);
	};
}
}
