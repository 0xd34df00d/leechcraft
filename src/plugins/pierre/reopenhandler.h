/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/iinfo.h>


namespace LC
{
namespace Pierre
{

class ReopenHandler
{
	ICoreProxy_ptr Proxy_;

	ReopenHandler ();
public:
	static ReopenHandler& Instance ();

	void SetCoreProxy (const ICoreProxy_ptr&);
	void Triggered ();
};

}
}
