/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_choosebackenddialog.h"

namespace LC
{
namespace Monocle
{
	class ChooseBackendDialog : public QDialog
	{
		Q_OBJECT

		Ui::ChooseBackendDialog Ui_;

		QList<QObject*> Backends_;
	public:
		ChooseBackendDialog (const QList<QObject*>&, QWidget* = 0);

		QObject* GetSelectedBackend () const;
		bool GetRememberChoice () const;
	};
}
}
