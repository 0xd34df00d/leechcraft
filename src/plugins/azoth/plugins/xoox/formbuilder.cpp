/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "formbuilder.h"
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class FieldHandler
	{
	public:
		virtual ~FieldHandler () {}
		
		void CreateWidget (const QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QWidget *w = CreateWidgetImpl (field, layout);
			if (!w)
				return;
			w->setToolTip (field.description ());
			w->setObjectName (field.key ());
		}
	protected:
		virtual QWidget* CreateWidgetImpl (const QXmppDataForm::Field&, QFormLayout*) = 0;
	};
	
	class BooleanHandler : public FieldHandler
	{
	protected:
		QWidget* CreateWidgetImpl (const QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QCheckBox *box = new QCheckBox (field.label ());
			box->setChecked (field.value ().toBool ());
			layout->addWidget (box);
			return box;
		}
	};

	FormBuilder::FormBuilder ()
	{
		Type2Handler_ [QXmppDataForm::Field::BooleanField].reset (new BooleanHandler);
	}
	
	QWidget* FormBuilder::CreateForm (const QXmppDataForm& form, QWidget *parent) const
	{
		if (form.isNull ())
			return 0;

		QWidget *widget = new QWidget (parent);
		widget->setWindowTitle (form.title ());

		QFormLayout *layout = new QFormLayout;
		widget->setLayout (layout);

		if (!form.title ().isEmpty ())
			layout->addRow (new QLabel (form.title ()));
		if (!form.instructions ().isEmpty ())
			layout->addRow (new QLabel (form.instructions ()));
		
		try
		{
			Q_FOREACH (const QXmppDataForm::Field& field, form.fields ())
			{
				if (!Type2Handler_.contains (field.type ()))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown field type"
							<< field.type ();
					continue;
				}
				Type2Handler_ [field.type ()]->CreateWidget (field, layout);
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ()
					<< "while trying to process fields";
			return 0;
		}

		return widget;
	}
}
}
}
