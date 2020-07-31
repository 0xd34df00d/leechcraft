/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_SHARERIEXDIALOG_H
#define PLUGINS_AZOTH_SHARERIEXDIALOG_H
#include <QDialog>
#include "ui_shareriexdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	class ShareRIEXDialog : public QDialog
	{
		Q_OBJECT

		Ui::ShareRIEXDialog Ui_;

		ICLEntry *Entry_;

		QStandardItemModel *Model_;
	public:
		ShareRIEXDialog (ICLEntry*, QWidget* = 0);

		QList<ICLEntry*> GetSelectedEntries () const;
		QString GetShareMessage () const;
		bool ShouldSuggestGroups () const;
	private slots:
		void fillModel ();
	};
}
}

#endif
