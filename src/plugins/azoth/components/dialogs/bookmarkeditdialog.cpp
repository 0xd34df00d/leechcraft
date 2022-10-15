/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarkeditdialog.h"
#include <QVBoxLayout>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/isupportbookmarks.h"
#include "interfaces/azoth/imucbookmarkeditorwidget.h"

namespace LC
{
namespace Azoth
{
	namespace
	{
		QWidget* GetEditWidget (IAccount *acc)
		{
			const auto isb = qobject_cast<ISupportBookmarks*> (acc->GetQObject ());
			return isb->GetMUCBookmarkEditorWidget ();
		}
	}

	BookmarkEditDialog::BookmarkEditDialog (IAccount *acc, QWidget *parent)
	: QDialog { parent }
	, EditorWidget_ { GetEditWidget (acc) }
	, EditorWidgetIface_ { qobject_cast<IMUCBookmarkEditorWidget*> (EditorWidget_) }
	{
		Ui_.setupUi (this);
		Ui_.MainLayout_->insertWidget (0, EditorWidget_);
	}

	BookmarkEditDialog::BookmarkEditDialog (const QVariantMap& data, IAccount *acc, QWidget *parent)
	: BookmarkEditDialog { acc, parent }
	{
		if (!data.isEmpty ())
			EditorWidgetIface_->SetIdentifyingData (data);
	}

	QVariantMap BookmarkEditDialog::GetIdentifyingData () const
	{
		return EditorWidgetIface_->GetIdentifyingData ();
	}
}
}
