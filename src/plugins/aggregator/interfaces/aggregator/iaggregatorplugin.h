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
namespace Aggregator
{
	class IProxyObject;

	class IAggregatorPlugin
	{
	protected:
		virtual ~IAggregatorPlugin () = default;
	public:
		virtual void InitPlugin (IProxyObject*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Aggregator::IAggregatorPlugin,
		"org.LeechCraft.Aggregator.IProxyObject/1.0")
