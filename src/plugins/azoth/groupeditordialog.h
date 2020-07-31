/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_GROUPEDITORDIALOG_H
#define PLUGINS_AZOTH_GROUPEDITORDIALOG_H
#include <QDialog>
#include "ui_groupeditordialog.h"

namespace LC
{
namespace Azoth
{
	class GroupEditorDialog : public QDialog
	{
		Q_OBJECT

		Ui::GroupEditorDialog Ui_;
	public:
		GroupEditorDialog (const QStringList& initial,
				const QStringList& all, QWidget* = 0);

		QStringList GetGroups () const;
	private slots:
		void on_GroupsSelector__tagsSelectionChanged (const QStringList&);
	};
}
}

#endif
