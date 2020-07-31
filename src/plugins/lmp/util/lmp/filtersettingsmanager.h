/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <xmlsettingsdialog/basesettingsmanager.h>
#include "lmputilconfig.h"

namespace LC
{
namespace LMP
{
	class LMP_UTIL_API FilterSettingsManager : public Util::BaseSettingsManager
	{
		Q_OBJECT
		const QString FilterId_;
	public:
		FilterSettingsManager (const QString& filterId, QObject* = nullptr);
	protected:
		QSettings* BeginSettings () const;
		void EndSettings (QSettings*) const;
	};
}
}
