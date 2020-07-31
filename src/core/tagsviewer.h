/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef TAGSVIEWER_H
#define TAGSVIEWER_H
#include <QWidget>
#include "ui_tagsviewer.h"

namespace LC
{
	class TagsViewer : public QWidget
	{
		Q_OBJECT

		Ui::TagsViewer Ui_;
	public:
		TagsViewer (QWidget* = 0);
	private slots:
		void on_Rename__released ();
		void on_Remove__released ();
	};
};

#endif

