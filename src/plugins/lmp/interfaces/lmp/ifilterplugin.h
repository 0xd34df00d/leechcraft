/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIcon>
#include <QtPlugin>
#include "ifilterelement.h"

namespace LC
{
namespace LMP
{
	struct EffectInfo
	{
		QByteArray ID_;
		QString Name_;
		QIcon Icon_;
		bool IsSingleton_;

		std::function<IFilterElement* (const QByteArray&, IPath*)> EffectFactory_;
	};

	class IFilterPlugin
	{
	public:
		virtual ~IFilterPlugin () {}

		virtual QList<EffectInfo> GetEffects () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::IFilterPlugin, "org.LeechCraft.LMP.IFilterPlugin/1.0")
