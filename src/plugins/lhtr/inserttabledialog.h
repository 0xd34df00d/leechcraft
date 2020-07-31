/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_inserttabledialog.h"

namespace LC
{
namespace LHTR
{
	class InsertTableDialog : public QDialog
	{
		Q_OBJECT

		Ui::InsertTableDialog Ui_;
	public:
		InsertTableDialog (QWidget* = 0);

		QString GetCaption () const;
		int GetRows () const;
		int GetColumns () const;
	};
}
}
