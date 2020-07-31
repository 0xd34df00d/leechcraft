/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_ACCEPTRIEXDIALOG_H
#define PLUGINS_AZOTH_ACCEPTRIEXDIALOG_H
#include <QDialog>
#include "interfaces/azoth/isupportriex.h"
#include "ui_acceptriexdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
	class AcceptRIEXDialog : public QDialog
	{
		Ui::AcceptRIEXDialog Ui_;

		QStandardItemModel *Model_;

		enum Column
		{
			Action,
			ID,
			Name,
			Groups
		};
	public:
		AcceptRIEXDialog (const QList<RIEXItem>&, QObject*, QString, QWidget* = 0);

		QList<RIEXItem> GetSelectedItems () const;
	};
}
}

#endif
