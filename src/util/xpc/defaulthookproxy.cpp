/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "defaulthookproxy.h"

namespace LC::Util
{
	DefaultHookProxy::DefaultHookProxy (QMap<QByteArray, QVariant> map)
	: Name2NewVal_ { std::move (map) }
	{
	}

	void DefaultHookProxy::CancelDefault ()
	{
		Cancelled_ = true;
	}

	bool DefaultHookProxy::IsCancelled () const
	{
		return Cancelled_;
	}

	const QVariant& DefaultHookProxy::GetReturnValue () const
	{
		return ReturnValue_;
	}

	void DefaultHookProxy::SetReturnValue (const QVariant& val)
	{
		ReturnValue_ = val;
	}

	QVariant DefaultHookProxy::GetValue (const QByteArray& name) const
	{
		return Name2NewVal_.value (name);
	}

	void DefaultHookProxy::SetValue (const QByteArray& name, const QVariant& val)
	{
		Name2NewVal_ [name] = val;
	}
}
