/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_OTZERKALU_OTZERKALUWIDGET_H
#define PLUGINS_OTZERKALU_OTZERKALUWIDGET_H
#include <QDialog>
#include <QUrl>
#include <QFileDialog>
#include "ui_otzerkaludialog.h"

namespace LC
{
namespace Otzerkalu
{
	class OtzerkaluDialog : public QDialog
	{
		Q_OBJECT
		Ui::OtzerkaluDialog Ui_;
	public:
		OtzerkaluDialog (QWidget *parent = 0);
		
		int GetRecursionLevel () const;
		QString GetDir () const;
		bool FetchFromExternalHosts () const;
	private slots:
		void on_ChooseDirButton__clicked ();
	};
}
}
#endif //PLUGINS_OTZERKALU_OTZERKALUWIDGET_H
