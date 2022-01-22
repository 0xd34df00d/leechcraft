/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_hyperlinkdialog.h"

namespace LC::LHTR
{
	class HyperlinkDialog : public QDialog
	{
		Ui::HyperlinkDialog Ui_;
	public:
		explicit HyperlinkDialog (QWidget* = nullptr);

		QString GetLink () const;
		QString GetText () const;
		QString GetTitle () const;
		QString GetTarget () const;
	};
}
