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

namespace LC
{
namespace Poshuku
{
	HistoryWidget::HistoryWidget (QWidget *parent)
	: QWidget { parent }
	, HistoryFilterModel_ { new Util::FixedStringFilterProxyModel { this } }
	{
		Ui_.setupUi (this);

		HistoryFilterModel_->setSourceModel (Core::Instance ().GetHistoryModel ());
		Ui_.HistoryView_->setModel (HistoryFilterModel_);

		connect (Ui_.HistoryFilterLine_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (updateHistoryFilter ()));
		connect (Ui_.HistoryFilterCaseSensitivity_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (updateHistoryFilter ()));

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

	void HistoryWidget::on_HistoryView__activated (const QModelIndex& index)
	{
		if (!index.parent ().isValid ())
			return;

		const auto& url = index.sibling (index.row (), HistoryModel::ColumnURL).data ().toString ();
		Core::Instance ().NewURL (url);
	}

	void HistoryWidget::updateHistoryFilter ()
	{
		HistoryFilterModel_->SetFilterString (Ui_.HistoryFilterLine_->text ());

		const auto cs = Ui_.HistoryFilterCaseSensitivity_->checkState () == Qt::Checked ?
				Qt::CaseSensitive :
				Qt::CaseInsensitive;
		HistoryFilterModel_->setFilterCaseSensitivity (cs);
	}
}
}
