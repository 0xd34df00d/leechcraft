/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_BOOKMARKEDITWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_BOOKMARKEDITWIDGET_H
#include <QWidget>
#include <QVariant>
#include <interfaces/azoth/imucbookmarkeditorwidget.h>
#include "ui_bookmarkeditwidget.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class BookmarkEditWidget : public QWidget
							 , public IMUCBookmarkEditorWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCBookmarkEditorWidget)

		Ui::BookmarkEditWidget Ui_;
	public:
		BookmarkEditWidget (QWidget* = 0);

		QVariantMap GetIdentifyingData () const;
		void SetIdentifyingData (const QVariantMap&);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_BOOKMARKEDITWIDGET_H
