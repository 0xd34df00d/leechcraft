/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "featurepermnotification.h"

namespace LC
{
namespace Poshuku
{
	FeaturePermNotification::FeaturePermNotification (const QString& text, QWidget *parent)
	: PageNotification { parent }
	{
		Ui_.setupUi (this);
		Ui_.Label_->setText (text);

		connect (Ui_.Grant_,
				SIGNAL (released ()),
				this,
				SIGNAL (granted ()));
		connect (Ui_.Deny_,
				SIGNAL (released ()),
				this,
				SIGNAL (denied ()));
	}
}
}
