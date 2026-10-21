/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/core/icoreproxyfwd.h>
#include <util/tags/tagseditoraddon.h>
#include "ui_searcherslist.h"

namespace LC
{
struct Entity;

namespace SeekThru
{
	class SearchersList : public QWidget
	{
		Q_OBJECT


		Ui::SearchersList Ui_;
		Util::TagsEditorAddon *SearcherTags_ = nullptr;
	public:
		explicit SearchersList (const ICoreProxy_ptr&, QWidget* = nullptr);
	private slots:
		void handleCurrentChanged (const QModelIndex&);
		void on_ButtonAdd__released ();
		void on_ButtonRemove__released ();
	};
}
}
