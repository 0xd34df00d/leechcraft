/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerpushbutton.h"
#include <QGridLayout>
#include <QPushButton>

namespace LC
{
	bool ItemHandlerPushButton::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "pushbutton";
	}

	void ItemHandlerPushButton::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QPushButton *button = new QPushButton (XSD_->GetWidget ());
		button->setObjectName (item.attribute ("name"));
		button->setText (XSD_->GetLabel (item));
		lay->addWidget (button, lay->rowCount (), 0, 1, 2);
		connect (button,
				SIGNAL (released ()),
				XSD_,
				SLOT (handlePushButtonReleased ()));
		button->setProperty ("SearchTerms", button->text ());
	}
}
