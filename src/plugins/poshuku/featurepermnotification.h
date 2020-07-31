/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/gui/pagenotification.h>
#include "ui_featurepermnotification.h"

namespace LC
{
namespace Poshuku
{
	class FeaturePermNotification : public Util::PageNotification
	{
		Q_OBJECT

		Ui::FeaturePermNotification Ui_;
	public:
		FeaturePermNotification (const QString& text, QWidget*);
	signals:
		void granted ();
		void denied ();
	};
}
}
