/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_mrimaccountconfigwidget.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	class MRIMAccountConfigWidget : public QWidget
	{
		Q_OBJECT

		Ui::MRIMAccountConfigWidget Ui_;
	public:
		MRIMAccountConfigWidget (QWidget* = 0);

		QString GetLogin () const;
		QString GetPassword () const;
	};
}
}
}
