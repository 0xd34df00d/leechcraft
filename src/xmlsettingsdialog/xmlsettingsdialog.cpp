#include <QFile>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <QtDebug>
#include "xmlsettingsdialog.h"

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
	QString locale = QLocale::system ().name ().toLower ();
	if (locale == "c")
		locale = "en";

	QString sectionTitle;
	QDomElement name = page.firstChildElement ("name");
	while (!name.isNull ())
	{
		if (name.attribute ("lang").toLower () == locale)
		{
			sectionTitle = name.attribute ("value");
			break;
		}
		name = name.nextSiblingElement ("name");
	}

	Sections_->addItem (sectionTitle);

	QWidget *baseWidget = new QWidget;
	Pages_->addWidget (baseWidget);
	baseWidget->setLayout (new QGridLayout);

	QDomElement item = page.firstChildElement ("item");
	while (!item.isNull ())
	{
		ParseItem (item, baseWidget);
		item = item.nextSiblingElement ("item");
	}
}

void XmlSettingsDialog::ParseItem (const QDomElement& item, QWidget *baseWidget)
{
	QString type = item.attribute ("type");

	QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
	int row = lay->rowCount ();

	if (type.isEmpty () || type.isNull ())
		return;
	else if (type == "lineedit")
	{
		QLabel *label = new QLabel (GetLabel (item));
		QLineEdit *edit = new QLineEdit (item.attribute ("default"));
		edit->setObjectName (item.attribute ("property"));
		connect (edit, SIGNAL (textChanged (const QString&)), this, SLOT (updatePreferences ()));

		lay->addWidget (label, row, 0);
		lay->addWidget (edit, row, 1);
	}
	else if (type == "checkbox")
	{
		QCheckBox *box = new QCheckBox (GetLabel (item));
		box->setObjectName (item.attribute ("property"));
		box->setCheckState (item.attribute ("state") == "on" ? Qt::Checked : Qt::Unchecked);
		connect (box, SIGNAL (stateChanged (int)), this, SLOT (updatePreferences ()));

		lay->addWidget (box, row, 0, 1, 2);
	}
}

QString XmlSettingsDialog::GetLabel (const QDomElement& item)
{
	QString locale = QLocale::system ().name ().toLower ();
	if (locale == "c")
		locale = "en";

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

