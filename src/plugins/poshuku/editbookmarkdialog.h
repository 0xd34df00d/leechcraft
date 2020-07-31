/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_EDITBOOKMARKDIALOG_H
#define PLUGINS_POSHUKU_EDITBOOKMARKDIALOG_H
#include <QDialog>
#include "ui_editbookmarkdialog.h"

namespace LC
{
namespace Poshuku
{
	class EditBookmarkDialog : public QDialog
	{
		Q_OBJECT

		Ui::EditBookmarkDialog Ui_;
	public:
		EditBookmarkDialog (const QModelIndex&, QWidget* = 0);

		QString GetURL () const;
		QString GetTitle () const;
		QStringList GetTags () const;
	};
}
}

#endif
