/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMainWindow>
#include "ui_sourceviewer.h"

namespace LC
{
namespace Poshuku
{
	class SourceViewer : public QMainWindow
	{
		Q_OBJECT

		Ui::SourceViewer Ui_;
	public:
		SourceViewer (QWidget* = 0);

		void SetHtml (const QString&);
	};
}
}
