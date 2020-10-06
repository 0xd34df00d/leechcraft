/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "callbackmanager.h"

namespace LC::Azoth::Sarin
{
	void CallbackManager::SetTox (const std::shared_ptr<Tox>& tox)
	{
		Tox_.WithWrite ([tox] (auto& instance) { instance = tox; });

		if (tox)
			ProxyReggers_.WithRead ([&] (const auto& reggers)
					{
						for (const auto& regger : reggers)
							regger (tox.get ());
					});
	}
}
