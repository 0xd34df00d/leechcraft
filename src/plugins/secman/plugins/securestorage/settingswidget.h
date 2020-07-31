/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QLineEdit>
#include "ui_settingswidget.h"

namespace LC
{
namespace SecMan
{
namespace SecureStorage
{
	class SettingsWidget : public QWidget
	{
		Q_OBJECT

		Ui::SettingsWidget Ui_;
	public:
		SettingsWidget ();

		QString GetOldPassword ();
		QString GetNewPassword ();
		void ClearPasswordFields ();
	signals:
		void clearSettingsRequested ();
		void changePasswordRequested ();
	};
}
}
}
