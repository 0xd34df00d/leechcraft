/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_SETSTATUSDIALOG_H
#define PLUGINS_AZOTH_SETSTATUSDIALOG_H
#include <QDialog>
#include "interfaces/azoth/azothcommon.h"
#include "ui_setstatusdialog.h"

namespace LC
{
namespace Azoth
{
	class SetStatusDialog : public QDialog
	{
		Q_OBJECT

		Ui::SetStatusDialog Ui_;
		QString Context_;

		enum Roles
		{
			ItemState = Qt::UserRole + 1,
			StateText
		};
	public:
		SetStatusDialog (const QString& context, QWidget* = 0);

		State GetState () const;
		QString GetStatusText () const;
	private slots:
		void save ();
		void on_StatusBox__currentIndexChanged ();
	};
}
}

#endif
