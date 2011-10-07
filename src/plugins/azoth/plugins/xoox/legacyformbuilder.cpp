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

#include "legacyformbuilder.h"
#include <boost/bind.hpp>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QLineEdit>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	namespace
	{
		void LineEditActorImpl (QWidget *form, const QXmppElement& elem,
				const QString& fieldLabel)
		{
			QLabel *label = new QLabel (fieldLabel);
			QLineEdit *edit = new QLineEdit (elem.value ());
			edit->setObjectName ("field");
			edit->setProperty ("FieldName", elem.tagName ());

			QHBoxLayout *lay = new QHBoxLayout (form);
			lay->addWidget (label);
			lay->addWidget (edit);
			qobject_cast<QVBoxLayout*> (form->layout ())->addLayout (lay);
		}

		void InstructionsActor (QWidget *form, const QXmppElement& elem)
		{
			QLabel *label = new QLabel (elem.value ());
			form->layout ()->addWidget (label);
		}
	}

	LegacyFormBuilder::LegacyFormBuilder ()
	: Widget_ (0)
	{
		Tag2Actor_ ["username"] = boost::bind (LineEditActorImpl,
				_1, _2, tr ("Username:"));
		Tag2Actor_ ["password"] = boost::bind (LineEditActorImpl,
				_1, _2, tr ("Password:"));
		Tag2Actor_ ["registered"] = boost::bind (LineEditActorImpl,
				_1, _2, tr ("Registered:"));
		Tag2Actor_ ["first"] = boost::bind (LineEditActorImpl,
				_1, _2, tr ("First name:"));
		Tag2Actor_ ["last"] = boost::bind (LineEditActorImpl,
				_1, _2, tr ("Last name:"));
		Tag2Actor_ ["nick"] = boost::bind (LineEditActorImpl,
				_1, _2, tr ("Nick:"));
		Tag2Actor_ ["email"] = boost::bind (LineEditActorImpl,
				_1, _2, tr ("E-Mail:"));
		Tag2Actor_ ["instructions"] = boost::bind (InstructionsActor,
				_1, _2);
	}

	QWidget* LegacyFormBuilder::CreateForm (const QXmppElement& containing,
			QWidget *parent)
	{
		Widget_ = new QWidget (parent);
		Widget_->setLayout (new QVBoxLayout ());

		QXmppElement element = containing.firstChildElement ();
		while (!element.isNull ())
		{
			const QString& tag = element.tagName ();

			if (!Tag2Actor_.contains (tag))
				qWarning () << Q_FUNC_INFO
						<< "unknown tag";
			else
				Tag2Actor_ [tag] (Widget_, element);

			element = element.nextSiblingElement ();
		}

		return Widget_;
	}

	QList<QXmppElement> LegacyFormBuilder::GetFilledChildren () const
	{
		QList<QXmppElement> result;
		if (!Widget_)
			return result;

		Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ("field"))
		{
			QXmppElement elem;
			elem.setTagName (edit->property ("FieldName").toString ());
			elem.setValue (edit->text ());
			result << elem;
		}

		return result;
	}

	QString LegacyFormBuilder::GetUsername () const
	{
		if (!Widget_)
			return QString ();

		Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ("field"))
			if (edit->property ("FieldName").toString () == "username")
				return edit->text ();

		return QString ();
	}

	QString LegacyFormBuilder::GetPassword () const
	{
		Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ("field"))
			if (edit->property ("FieldName").toString () == "password")
				return edit->text ();

		return QString ();
	}
}
}
}
