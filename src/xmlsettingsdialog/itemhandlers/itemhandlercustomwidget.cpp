/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlercustomwidget.h"
#include <QGridLayout>
#include <QLabel>

namespace LC
{
	bool ItemHandlerCustomWidget::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "customwidget";
	}

	void ItemHandlerCustomWidget::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QWidget *widget = new QWidget (XSD_->GetWidget ());
		widget->setObjectName (item.attribute ("name"));
		QVBoxLayout *layout = new QVBoxLayout ();
		layout->setContentsMargins (0, 0, 0, 0);
		widget->setLayout (layout);
		widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		const auto rc = lay->rowCount ();
		lay->setRowStretch (rc, 1);
		lay->addWidget (widget, rc, 0, 1, -1);
	}
}
