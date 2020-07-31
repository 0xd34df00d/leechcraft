/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_DRAWATTENTIONDIALOG_H
#define PLUGINS_AZOTH_DRAWATTENTIONDIALOG_H
#include <QDialog>
#include "ui_drawattentiondialog.h"

namespace LC
{
namespace Azoth
{
	class DrawAttentionDialog : public QDialog
	{
		Ui::DrawAttentionDialog Ui_;
	public:
		DrawAttentionDialog (const QStringList&, QWidget* = 0);
		
		QString GetResource () const;
		QString GetText () const;
	};
}
}

#endif
