/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_accregisterdetailspage.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class AccRegisterDetailsPage : public QWidget
	{
		Q_OBJECT

		Ui::AccRegisterDetailsPage Ui_;
	public:
		AccRegisterDetailsPage (QWidget* = 0);

		QString GetId () const;
		QString GetNickname () const;
	private slots:
		void on_GenerateButton__released ();
	};
}
}
}
