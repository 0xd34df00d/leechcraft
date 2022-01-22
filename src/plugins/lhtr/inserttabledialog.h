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

namespace LC::LHTR
{
	class InsertTableDialog : public QDialog
	{
		Ui::InsertTableDialog Ui_;
	public:
		explicit InsertTableDialog (QWidget* = nullptr);

		QString GetCaption () const;
		int GetRows () const;
		int GetColumns () const;
	};
}
