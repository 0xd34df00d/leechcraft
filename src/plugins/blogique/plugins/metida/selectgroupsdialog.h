/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_selectgroupsdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJProfile;

	class SelectGroupsDialog : public QDialog
	{
		Q_OBJECT

		Ui::SelectGroupsDialog Ui_;
		QStandardItemModel *Model_;
	public:
		SelectGroupsDialog (LJProfile *profile, quint32 allowMask, QWidget *parent = 0);

		QList<uint> GetSelectedGroupsIds () const;
		
		void SetHeaderLabel (const QString& text);
	};
}
}
}

