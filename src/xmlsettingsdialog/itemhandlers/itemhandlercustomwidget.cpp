/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "itemhandlercustomwidget.h"
#include <QFormLayout>
#include <QLabel>

namespace LeechCraft
{
	ItemHandlerCustomWidget::ItemHandlerCustomWidget ()
	{
	}

	ItemHandlerCustomWidget::~ItemHandlerCustomWidget ()
	{
	}

	bool ItemHandlerCustomWidget::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "customwidget";
	}

	void ItemHandlerCustomWidget::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		QWidget *widget = new QWidget (XSD_);
		widget->setObjectName (item.attribute ("name"));
		QVBoxLayout *layout = new QVBoxLayout ();
		layout->setContentsMargins (0, 0, 0, 0);
		widget->setLayout (layout);
		widget->setSizePolicy (QSizePolicy::Expanding,
				QSizePolicy::Expanding);

		if (item.attribute ("label") == "own")
			lay->addRow (widget);
		else
		{
			QLabel *label = new QLabel (XSD_->GetLabel (item));
			label->setWordWrap (false);

			lay->addRow (label, widget);
		}
	}
};
