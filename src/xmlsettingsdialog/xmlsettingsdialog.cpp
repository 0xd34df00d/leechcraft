#include <QFile>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <QtDebug>
#include "xmlsettingsdialog.h"
#include "rangewidget.h"

XmlSettingsDialog::XmlSettingsDialog (QWidget *parent)
: QDialog (parent)
{
	Pages_ = new QStackedWidget;
	Sections_ = new QListWidget;
	Sections_->setMinimumWidth (100);
	Sections_->setMaximumWidth (150);

	connect (Sections_, SIGNAL (currentRowChanged (int)), Pages_, SLOT (setCurrentIndex (int)));

	OK_ = new QPushButton (tr ("OK"));
	Cancel_ = new QPushButton (tr ("Cancel"));

	QHBoxLayout *buttons = new QHBoxLayout;
	buttons->addStretch (1);
	buttons->addWidget (OK_);
	buttons->addWidget (Cancel_);
	connect (OK_, SIGNAL (released ()), this, SLOT (accept ()));
	connect (Cancel_, SIGNAL (released ()), this, SLOT (reject ()));

	QVBoxLayout *rightLay = new QVBoxLayout;
	QHBoxLayout *mainLay = new QHBoxLayout (this);
	mainLay->addWidget (Sections_);
	rightLay->addWidget (Pages_);
	rightLay->addStretch (1);
	mainLay->addLayout (rightLay);
	rightLay->addLayout (buttons);
	setLayout (mainLay);
}

void XmlSettingsDialog::RegisterObject (QObject* obj, const QString& filename)
{
	WorkingObject_ = obj;
	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
	{
		qWarning () << "cannot open file";
		return;
	}
	QByteArray data = file.readAll ();
	file.close ();

	QDomDocument document;
	if (!document.setContent (data))
	{
		qWarning () << "Could not parse file";
		return;
	}
	QDomElement root = document.documentElement ();
	if (root.tagName () != "settings")
	{
		qWarning () << "Bad settings file";
		return;
	}

	QDomElement pageChild = root.firstChildElement ("page");
	while (!pageChild.isNull ())
	{
		ParsePage (pageChild);
		pageChild = pageChild.nextSiblingElement ("page");
		adjustSize ();
	}
}

void XmlSettingsDialog::ParsePage (const QDomElement& page)
{
	QString sectionTitle = GetLabel (page);
	Sections_->addItem (sectionTitle);

	QWidget *baseWidget = new QWidget;
	Pages_->addWidget (baseWidget);
	QGridLayout *lay = new QGridLayout;
	baseWidget->setLayout (lay);

	ParseEntity (page, baseWidget);
	lay->setRowStretch (lay->rowCount (), 1);
}

void XmlSettingsDialog::ParseEntity (const QDomElement& entity, QWidget *baseWidget)
{
	QDomElement item = entity.firstChildElement ("item");
	while (!item.isNull ())
	{
		ParseItem (item, baseWidget);
		item = item.nextSiblingElement ("item");
	}

	QDomElement gbox = entity.firstChildElement ("groupbox");
	while (!gbox.isNull ())
	{
		QGroupBox *box = new QGroupBox (GetLabel (gbox));
		box->setLayout (new QGridLayout);
		ParseEntity (gbox, box);
		
		QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
		int row = lay->rowCount ();
		lay->addWidget (box, row, 0, 1, 2);

		gbox = gbox.nextSiblingElement ("groupbox");
	}

	QDomElement tab = entity.firstChildElement ("tab");
	if (!tab.isNull ())
	{
		QTabWidget *tabs = new QTabWidget;
		QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
		int row = lay->rowCount ();
		lay->addWidget (tabs, row, 0, 1, 2);
		while (!tab.isNull ())
		{
			QWidget *page = new QWidget;
			QGridLayout *widgetLay = new QGridLayout;
			page->setLayout (widgetLay);
			tabs->addTab (page, GetLabel (tab));
			ParseEntity (tab, page);
			tab = tab.nextSiblingElement ("tab");
			widgetLay->setRowStretch (widgetLay->rowCount (), 1);
		}
	}
}

void XmlSettingsDialog::ParseItem (const QDomElement& item, QWidget *baseWidget)
{
	QString type = item.attribute ("type");

	QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
	int row = lay->rowCount ();

	QString property = item.attribute ("property");
	QVariant value = WorkingObject_->property (property.toLatin1 ().constData ());
	if (!value.isValid () || value.isNull () && item.hasAttribute ("default"))
	{
		value = item.attribute ("default");
		WorkingObject_->setProperty (property.toLatin1 ().constData (), value);
	}

	if (type.isEmpty () || type.isNull ())
		return;
	else if (type == "lineedit")
	{
		QLabel *label = new QLabel (GetLabel (item));

		QLineEdit *edit = new QLineEdit (value.toString ());
		edit->setObjectName (property);
		if (item.hasAttribute ("password"))
			edit->setEchoMode (QLineEdit::Password);
		connect (edit, SIGNAL (textChanged (const QString&)), this, SLOT (updatePreferences ()));

		lay->addWidget (label, row, 0);
		lay->addWidget (edit, row, 1);
	}
	else if (type == "checkbox")
	{
		QCheckBox *box = new QCheckBox (GetLabel (item));
		box->setObjectName (property);
		if (!value.isValid () || value.isNull ())
		{
			value = (item.attribute ("state") == "on");
			WorkingObject_->setProperty (property.toLatin1 ().constData (), value);
		}
		box->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
		connect (box, SIGNAL (stateChanged (int)), this, SLOT (updatePreferences ()));

		lay->addWidget (box, row, 0, 1, 2);
	}
	else if (type == "spinbox")
	{
		QLabel *label = new QLabel (GetLabel (item));
		QSpinBox *box = new QSpinBox;
		box->setObjectName (property);
		if (item.hasAttribute ("minimum"))
			box->setMinimum (item.attribute ("minimum").toInt ());
		if (item.hasAttribute ("maximum"))
			box->setMaximum (item.attribute ("maximum").toInt ());
		if (item.hasAttribute ("step"))
			box->setSingleStep (item.attribute ("step").toInt ());
		if (item.hasAttribute ("suffix"))
			box->setSuffix (item.attribute ("suffix"));
		box->setValue (value.toInt ());
		connect (box, SIGNAL (valueChanged (int)), this, SLOT (updatePreferences ()));
		
		lay->addWidget (label, row, 0);
		lay->addWidget (box, row, 1);
	}
	else if (type == "groupbox" && item.attribute ("checkable") == "true")
	{
		QGroupBox *box = new QGroupBox (GetLabel (item));
		box->setObjectName (property);
		box->setLayout (new QGridLayout);
		box->setCheckable (true);
		if (!value.isValid () || value.isNull ())
		{
			value = (item.attribute ("state") == "on");
			WorkingObject_->setProperty (property.toLatin1 ().constData (), value);
		}
		box->setChecked (value.toBool ());
		connect (box, SIGNAL (toggled (bool)), this, SLOT (updatePreferences ()));
		ParseEntity (item, box);
		
		lay->addWidget (box, row, 0, 1, 2);
	}
	else if (type == "spinboxrange")
	{
		if (!value.isValid () || value.isNull () || !value.canConvert<QList<QVariant> > ())
		{
			QStringList parts = item.attribute ("default").split (":");
			QList<QVariant> result;
			if (parts.size () != 2)
			{
				qWarning () << "spinboxrange parse error, wrong default value";
				return;
			}
			result << parts.at (0).toInt () << parts.at (1).toInt ();
			value = result;
			WorkingObject_->setProperty (property.toLatin1 ().constData (), value);
		}

		QLabel *label = new QLabel (GetLabel (item));
		RangeWidget *widget = new RangeWidget ();
		widget->setObjectName (property);
		widget->SetMinimum (item.attribute ("minimum").toInt ());
		widget->SetMaximum (item.attribute ("maximum").toInt ());
		widget->SetRange (value);
		connect (widget, SIGNAL (changed ()), this, SLOT (updatePreferences ()));

		lay->addWidget (label, row, 0);
		lay->addWidget (widget, row, 1);
	}
	else
	{
		qWarning () << Q_FUNC_INFO << "unhandled type" << type;
	}
}

QString XmlSettingsDialog::GetLabel (const QDomElement& item)
{
	QString locale = QLocale::system ().name ().toLower ();
	if (locale == "c")
		locale = "en";

	locale = locale.left (2);

	QString result;
	QDomElement label = item.firstChildElement ("label");
	while (!label.isNull ())
	{
		if (label.attribute ("lang").toLower () == locale)
		{
			result = label.attribute ("value");
			break;
		}
		label = label.nextSiblingElement ("label");
	}
	return result;
}

void XmlSettingsDialog::updatePreferences ()
{
	QString propertyName = sender ()->objectName ();
	QVariant value;

	QLineEdit *edit = qobject_cast<QLineEdit*> (sender ());
	QCheckBox *checkbox = qobject_cast<QCheckBox*> (sender ());
	QSpinBox *spinbox = qobject_cast<QSpinBox*> (sender ());
	QGroupBox *groupbox = qobject_cast<QGroupBox*> (sender ());
	RangeWidget *rangeWidget = qobject_cast<RangeWidget*> (sender ());
	if (edit)
		value = edit->text ();
	else if (checkbox)
		value = checkbox->checkState ();
	else if (spinbox)
		value = spinbox->value ();
	else if (groupbox)
		value = groupbox->isChecked ();
	else if (rangeWidget)
		value = rangeWidget->GetRange ();
	else
	{
		qWarning () << Q_FUNC_INFO << "unhandled class" << sender ();
		return;
	}

	Prop2NewValue_ [propertyName] = value;
}

void XmlSettingsDialog::accept ()
{
	for (Property2Value_t::const_iterator i = Prop2NewValue_.begin (); i != Prop2NewValue_.end (); ++i)
		WorkingObject_->setProperty (i.key ().toLatin1 ().constData (), i.value ());

	QDialog::accept ();
}

