/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_radiocustomdialog.h"

namespace LC
{
namespace LMP
{
	class RadioCustomDialog : public QDialog
	{
		Q_OBJECT

		Ui::RadioCustomDialog Ui_;
	public:
		RadioCustomDialog (QWidget* = 0);

		QUrl GetUrl () const;
		void SetUrl (const QUrl&);

		QString GetName () const;
		void SetName (const QString&);
	};
}
}
