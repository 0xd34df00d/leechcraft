/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "backend.h"

namespace LC::HotSensors
{
	struct StoredTemp;

	class LmSensorsBackend : public Backend
	{
		QVector<StoredTemp> Features_;
	public:
		LmSensorsBackend (QObject* = nullptr);
		~LmSensorsBackend () override;

		void update () override;
	private:
		void EnumerateSensors ();
	};
}
