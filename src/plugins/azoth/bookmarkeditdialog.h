/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_bookmarkeditdialog.h"

namespace LC
{
namespace Azoth
{
	class IAccount;
	class IMUCBookmarkEditorWidget;

	class BookmarkEditDialog : public QDialog
	{
		Q_OBJECT

		Ui::BookmarkEditDialog Ui_;

		QWidget * const EditorWidget_;
		IMUCBookmarkEditorWidget * const EditorWidgetIface_;
	public:
		BookmarkEditDialog (IAccount*, QWidget* = nullptr);
		BookmarkEditDialog (const QVariantMap&, IAccount*, QWidget* = nullptr);

		QVariantMap GetIdentifyingData () const;
	};
}
}
