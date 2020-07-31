/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hookproxywrapper.h"
#include <interfaces/core/ihookproxy.h>

namespace LC
{
namespace Qrosp
{
	HookProxyWrapper::HookProxyWrapper (IHookProxy_ptr proxy)
	: Proxy_ (proxy)
	{
	}

	void HookProxyWrapper::CancelDefault ()
	{
		Proxy_->CancelDefault ();
	}

	const QVariant& HookProxyWrapper::GetReturnValue () const
	{
		return Proxy_->GetReturnValue ();
	}

	void HookProxyWrapper::SetReturnValue (const QVariant& val)
	{
		Proxy_->SetReturnValue (val);
	}

	void HookProxyWrapper::SetValue (const QByteArray& name, const QVariant& value)
	{
		Proxy_->SetValue (name, value);
	}
}
}
