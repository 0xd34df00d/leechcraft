/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace LMP
{
	class ILMPProxy;

	using ILMPProxy_ptr = ILMPProxy*;

	class ILMPPlugin
	{
	public:
		virtual ~ILMPPlugin () {}

		virtual void SetLMPProxy (ILMPProxy_ptr) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::ILMPPlugin, "org.LeechCraft.LMP.ILMPPlugin/1.0")
