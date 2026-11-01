/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_tagsviewer.h"

namespace LC
{
	class TagsViewer : public QWidget
	{
		Q_OBJECT

		Ui::TagsViewer Ui_;
	public:
		explicit TagsViewer (QWidget* = nullptr);
	private slots:
		void on_Modify__released ();
		void on_Remove__released ();
	};
}
