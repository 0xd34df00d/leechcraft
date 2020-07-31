/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QElapsedTimer>
#include "loadprocessbase.h"

namespace LC
{
	class LoadProcess : public LoadProcessBase
	{
		const QString Title_;
		const int Min_;
		const int Max_;
		int Value_ = 0;

		QElapsedTimer LastReport_;
	public:
		LoadProcess (const QString&, int, int);

		QString GetTitle () const override;
		int GetMin () const override;
		int GetMax () const override;
		int GetValue () const override;

		void ReportValue (int value) override;
	};
}
