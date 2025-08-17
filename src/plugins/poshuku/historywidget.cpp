/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historywidget.h"
#include <QDateTime>
#include <util/models/fixedstringfilterproxymodel.h>
#include "core.h"
#include "historymodel.h"

namespace LC::Poshuku
{
	HistoryWidget::HistoryWidget (QWidget *parent)
	: QWidget { parent }
	, HistoryFilterModel_ { new Util::FixedStringFilterProxyModel { this } }
	{
		Ui_.setupUi (this);

		connect (Ui_.HistoryView_,
				&QTreeView::activated,
				this,
				[] (const QModelIndex& index)
				{
					const auto& url = index.data (HistoryModel::Roles::URL).toString ();
					if (!url.isEmpty ())
						Core::Instance ().NewURL (url);
				});

		HistoryFilterModel_->setSourceModel (Core::Instance ().GetHistoryModel ());
		Ui_.HistoryView_->setModel (HistoryFilterModel_);

		connect (Ui_.HistoryFilterLine_,
				&QLineEdit::textChanged,
				HistoryFilterModel_,
				&Util::FixedStringFilterProxyModel::SetFilterString);
		connect (Ui_.HistoryFilterCaseSensitivity_,
				&QCheckBox::checkStateChanged,
				this,
				[this] (Qt::CheckState state)
				{
					const auto cs = state == Qt::Checked ? Qt::CaseSensitive : Qt::CaseInsensitive;
					HistoryFilterModel_->setFilterCaseSensitivity (cs);
				});

		const auto itemsHeader = Ui_.HistoryView_->header ();
		const auto& fm = fontMetrics ();
		itemsHeader->resizeSection (0,
				fm.horizontalAdvance ("Average site title can be very big, it's also the "
					"most important part, so it's priority is the biggest."));
		itemsHeader->resizeSection (1,
				fm.horizontalAdvance (QDateTime::currentDateTime ().toString () + " space"));
		itemsHeader->resizeSection (2,
				fm.horizontalAdvance ("Average URL could be very very long, but we don't account this."));
	}
}
