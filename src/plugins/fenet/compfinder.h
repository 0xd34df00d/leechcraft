/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once
#include "finderbase.h"
#include "compinfo.h"

namespace LC::Fenet
{
	class CompFinder : public FinderBase<CompInfo>
	{
	public:
		explicit CompFinder (QObject* = nullptr);

		CompInfo GetInfo (const QString&, const QStringList&, const QVariantMap&) const override;
	};
}
