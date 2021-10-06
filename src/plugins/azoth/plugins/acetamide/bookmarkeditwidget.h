/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QVariant>
#include <interfaces/azoth/imucbookmarkeditorwidget.h>
#include "ui_bookmarkeditwidget.h"

namespace LC::Azoth::Acetamide
{
	class BookmarkEditWidget : public QWidget
							 , public IMUCBookmarkEditorWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCBookmarkEditorWidget)

		Ui::BookmarkEditWidget Ui_;
	public:
		explicit BookmarkEditWidget (QWidget* = nullptr);

		QVariantMap GetIdentifyingData () const override;
		void SetIdentifyingData (const QVariantMap&) override;
	};
}
