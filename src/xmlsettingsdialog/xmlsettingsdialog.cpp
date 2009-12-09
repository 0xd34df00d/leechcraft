/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "xmlsettingsdialog.h"
#include <stdexcept>
#include <QFile>
#include <QtDebug>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QDir>
#include <QCheckBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QFormLayout>
#include <QRadioButton>
#include <QApplication>
#include <QUrl>
#include <QScrollArea>
#include <QDomNodeList>
#include <QtScript>
#include "rangewidget.h"
#include "filepicker.h"
#include "radiogroup.h"
#include "scripter.h"
#include "fontpicker.h"
#include "colorpicker.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

XmlSettingsDialog::XmlSettingsDialog ()
: Document_ (new QDomDocument)
{
	Pages_ = new QStackedWidget (this);

	QHBoxLayout *mainLay = new QHBoxLayout (this);
	mainLay->setContentsMargins (0, 0, 0, 0);
	mainLay->addWidget (Pages_);
	setLayout (mainLay);

	DefaultLang_ = "en";
}

XmlSettingsDialog::~XmlSettingsDialog ()
{
}

void XmlSettingsDialog::RegisterObject (QObject* obj, const QString& basename)
{
	WorkingObject_ = obj;
	QString filename;
	if (QFile::exists (basename))
		filename = basename;
	else if (QFile::exists (QString (":/") + basename))
		filename = QString (":/") + basename;
#ifdef Q_WS_WIN
	else if (QFile::exists (QString ("settings/") + basename))
		filename = QString ("settings/") + basename;
#else
	else if (QFile::exists (QString ("/usr/local/share/leechcraft/settings/") + basename))
		filename = QString ("/usr/local/share/leechcraft/settings/") + basename;
	else if (QFile::exists (QString ("/usr/share/leechcraft/settings/") + basename))
		filename = QString ("/usr/share/leechcraft/settings/") + basename;
#endif
	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
	{
		qWarning () << "cannot open file"
			<< filename
			<< basename;
		return;
	}
	QByteArray data = file.readAll ();
	file.close ();

	if (!Document_->setContent (data))
	{
		qWarning () << "Could not parse file";
		return;
	}
	QDomElement root = Document_->documentElement ();
	if (root.tagName () != "settings")
	{
		qWarning () << "Bad settings file";
		return;
	}

	QDomElement declaration = root.firstChildElement ("declare");
	while (!declaration.isNull ())
	{
		HandleDeclaration (declaration);
		declaration = declaration.nextSiblingElement ("declare");
	}

	QDomElement pageChild = root.firstChildElement ("page");
	while (!pageChild.isNull ())
	{
		ParsePage (pageChild);
		pageChild = pageChild.nextSiblingElement ("page");
	}

	obj->installEventFilter (this);

	UpdateXml (true);
}

QString XmlSettingsDialog::GetXml () const
{
	return Document_->toString ();
}

void XmlSettingsDialog::MergeXml (const QByteArray& newXml)
{
	QDomDocument newDoc;
	newDoc.setContent (newXml);

	QList<QByteArray> props = WorkingObject_->dynamicPropertyNames ();

	QDomNodeList nodes = newDoc.elementsByTagName ("item");
	for (int i = 0; i < nodes.size (); ++i)
	{
		QDomElement elem = nodes.at (i).toElement ();
		if (elem.isNull ())
			continue;

		QString propName = elem.attribute ("property");
		if (!props.contains (propName.toLatin1 ()))
			continue;

		QVariant value = GetValue (elem);
		if (value.isNull ())
			continue;

		WorkingObject_->setProperty (propName.toLatin1 ().constData (), value);
		Prop2NewValue_ [propName] = value;
	}

	UpdateXml ();
	Prop2NewValue_.clear ();
}

void XmlSettingsDialog::SetCustomWidget (const QString& name, QWidget *widget)
{
	QList<QWidget*> widgets = findChildren<QWidget*> (name);
	if (!widgets.size ())
		throw std::runtime_error (qPrintable (QString ("Widget %1 not "
						"found").arg (name)));
	if (widgets.size () > 1)
		throw std::runtime_error (qPrintable (QString ("Widget %1 "
						"appears to exist more than once").arg (name)));

	widgets.at (0)->layout ()->addWidget (widget);
	Customs_ << widget;
	connect (widget,
			SIGNAL (destroyed (QObject*)),
			this,
			SLOT (handleCustomDestroyed ()));
}

void XmlSettingsDialog::SetPage (int page)
{
	Pages_->setCurrentIndex (page);
}

QStringList XmlSettingsDialog::GetPages () const
{
	return Titles_;
}

void XmlSettingsDialog::HandleDeclaration (const QDomElement& decl)
{
	if (decl.hasAttribute ("defaultlang"))
		DefaultLang_ = decl.attribute ("defaultlang");
}

void XmlSettingsDialog::ParsePage (const QDomElement& page)
{
	QString sectionTitle = GetLabel (page);
	Titles_ << sectionTitle;

	QWidget *baseWidget = new QWidget;
	Pages_->addWidget (baseWidget);
	QFormLayout *lay = new QFormLayout;
	lay->setRowWrapPolicy (QFormLayout::DontWrapRows);
	lay->setFieldGrowthPolicy (QFormLayout::AllNonFixedFieldsGrow);
	lay->setContentsMargins (0, 0, 0, 0);
	baseWidget->setLayout (lay);
	baseWidget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	ParseEntity (page, baseWidget);
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
		QFormLayout *groupLayout = new QFormLayout ();
		groupLayout->setRowWrapPolicy (QFormLayout::DontWrapRows);
		groupLayout->setFieldGrowthPolicy (QFormLayout::AllNonFixedFieldsGrow);
		groupLayout->setContentsMargins (2, 2, 2, 2);
		box->setLayout (groupLayout);
		box->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
		ParseEntity (gbox, box);
		
		QFormLayout *lay = qobject_cast<QFormLayout*> (baseWidget->layout ());
		lay->addRow (box);

		gbox = gbox.nextSiblingElement ("groupbox");
	}

	QDomElement scroll = entity.firstChildElement ("scrollarea");
	while (!scroll.isNull ())
	{
		QScrollArea *area = new QScrollArea ();
		if (scroll.hasAttribute ("horizontalScroll"))
		{
			QString attr = scroll.attribute ("horizontalScroll");
			if (attr == "on")
				area->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
			else if (attr == "off")
				area->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		}
		if (scroll.hasAttribute ("verticalScroll"))
		{
			QString attr = scroll.attribute ("verticalScroll");
			if (attr == "on")
				area->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
			else if (attr == "off")
				area->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		}

		QFrame *areaWidget = new QFrame;
		QFormLayout *areaLayout = new QFormLayout;
		areaLayout->setRowWrapPolicy (QFormLayout::DontWrapRows);
		areaLayout->setFieldGrowthPolicy (QFormLayout::AllNonFixedFieldsGrow);
		areaWidget->setLayout (areaLayout);

		ParseEntity (scroll, areaWidget);
		area->setWidget (areaWidget);
		areaWidget->show ();

		QFormLayout *lay = qobject_cast<QFormLayout*> (baseWidget->layout ());
		lay->addRow (area);

		scroll = scroll.nextSiblingElement ("scrollarea");
	}

	QDomElement tab = entity.firstChildElement ("tab");
	if (!tab.isNull ())
	{
		QTabWidget *tabs = new QTabWidget;
		QFormLayout *lay = qobject_cast<QFormLayout*> (baseWidget->layout ());
		lay->addRow (tabs);
		while (!tab.isNull ())
		{
			QWidget *page = new QWidget;
			QFormLayout *widgetLay = new QFormLayout;
			widgetLay->setRowWrapPolicy (QFormLayout::DontWrapRows);
			widgetLay->setFieldGrowthPolicy (QFormLayout::AllNonFixedFieldsGrow);
			widgetLay->setContentsMargins (0, 0, 0, 0);
			page->setLayout (widgetLay);
			page->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
			tabs->addTab (page, GetLabel (tab));
			ParseEntity (tab, page);
			tab = tab.nextSiblingElement ("tab");
		}
	}
}

void XmlSettingsDialog::ParseItem (const QDomElement& item, QWidget *baseWidget)
{
	QString type = item.attribute ("type");

	QFormLayout *lay = qobject_cast<QFormLayout*> (baseWidget->layout ());

	QString property = item.attribute ("property");

	if (type.isEmpty () || type.isNull ())
		return;
	else if (type == "lineedit")
		DoLineedit (item, lay);
	else if (type == "checkbox")
		DoCheckbox (item, lay);
	else if (type == "spinbox")
		DoSpinbox (item, lay);
	else if (type == "doublespinbox")
		DoDoubleSpinbox (item, lay);
	else if (type == "groupbox" &&
			item.attribute ("checkable") == "true")
		DoGroupbox (item, lay);
	else if (type == "spinboxrange")
		DoSpinboxRange (item, lay);
	else if (type == "path")
		DoPath (item, lay);
	else if (type == "radio")
		DoRadio (item, lay);
	else if (type == "combobox")
		DoCombobox (item, lay);
	else if (type == "font")
		DoFont (item, lay);
	else if (type == "color")
		DoColor (item, lay);
	else if (type == "pushbutton")
		DoPushButton (item, lay);
	else if (type == "customwidget")
		DoCustomWidget (item, lay);
	else
		qWarning () << Q_FUNC_INFO << "unhandled type" << type;

	WorkingObject_->setProperty (property.toLatin1 ().constData (), GetValue (item));
}

QString XmlSettingsDialog::GetLabel (const QDomElement& item) const
{
	QString locale = QString(::getenv ("LANG")).left (2);
	if (locale.isNull () || locale.isEmpty ())
		locale = QLocale::system ().name ().toLower ();
	if (locale == "c")
		locale = "en";

	locale = locale.left (2);

	QString result = "<no label>";
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
	if (result == "<no label>")
	{
		label = item.firstChildElement ("label");
		while (!label.isNull ())
		{
			if (label.attribute ("lang").toLower () == DefaultLang_)
			{
				result = label.attribute ("value");
				break;
			}
			label = label.nextSiblingElement ("label");
		}
	}
	return result;
}

XmlSettingsDialog::LangElements XmlSettingsDialog::GetLangElements (const QDomElement& parent) const
{
	QString locale = QString(::getenv ("LANG")).left (2);
	if (locale.isNull () || locale.isEmpty ())
		locale = QLocale::system ().name ().toLower ();
	if (locale == "c")
		locale = "en";

	locale = locale.left (2);
	LangElements returning;
	returning.Valid_ = false;

	bool found = false;

	QDomElement result = parent.firstChildElement ("lang");
	while (!result.isNull ())
	{
		if (result.attribute ("value").toLower () == locale)
		{
			found = true;
			break;
		}
		result = result.nextSiblingElement ("lang");
	}
	if (!found)
	{
		result = parent.firstChildElement ("lang");
		while (!result.isNull ())
		{
			if (result.attribute ("value").toLower () == DefaultLang_)
			{
				found = true;
				break;
			}
			result = result.nextSiblingElement ("lang");
		}
	}
	if (!found)
	{
		result = parent.firstChildElement ("lang");
		while (!result.isNull ())
		{
			if (result.attribute ("value").toLower () == "en" || !result.hasAttribute ("value"))
			{
				found = true;
				break;
			}
			result = result.nextSiblingElement ("lang");
		}
	}
	if (result.isNull ())
		return returning;

	returning.Valid_ = true;

	QDomElement label = result.firstChildElement ("label");
	if (!label.isNull () && label.hasAttribute ("value"))
	{
		returning.Label_.first = true;
		returning.Label_.second = label.attribute ("value");
	}
	else
		returning.Label_.first = false;

	QDomElement suffix = result.firstChildElement ("suffix");
	if (!suffix.isNull () && suffix.hasAttribute ("value"))
	{
		returning.Suffix_.first = true;
		returning.Suffix_.second = suffix.attribute ("value");
	}
	else
		returning.Suffix_.first = false;

	return returning;
}

QVariant XmlSettingsDialog::GetValue (const QDomElement& item, bool ignoreObject) const
{
	QString type = item.attribute ("type");
	QString property = item.attribute ("property");

	QVariant value;
	if (ignoreObject)
		value = item.attribute ("default");
	else
	{
		QVariant tmpValue = WorkingObject_->property (property.toLatin1 ().constData ());
		if (tmpValue.isValid ())
			return tmpValue;
	}

	if (type == "lineedit" ||
			type == "spinbox" ||
			type == "doublespinbox")
		return item.attribute ("default");
	else if (type == "checkbox" ||
			(type == "groupbox" && item.attribute ("checkable") == "true"))
	{
		if (!value.isValid () ||
				value.isNull () ||
				value.toString () == "on" ||
				value.toString () == "off")
		{
			if (item.hasAttribute ("default"))
				value = (item.attribute ("default") == "on");
			else
				value = (item.attribute ("state") == "on");
		}
	}
	else if (type == "spinboxrange")
	{
		if (!value.isValid () ||
				value.isNull () ||
				!value.canConvert<QList<QVariant> > ())
		{
			QStringList parts = item.attribute ("default").split (":");
			QList<QVariant> result;
			if (parts.size () != 2)
			{
				qWarning () << "spinboxrange parse error, wrong default value";
				return QVariant ();
			}
			result << parts.at (0).toInt () << parts.at (1).toInt ();
			value = result;
		}
	}
	else if (type == "path")
	{
		if (value.isNull () ||
				value.toString ().isEmpty ())
			if (item.hasAttribute ("defaultHomePath") &&
					item.attribute ("defaultHomePath") == "true")
				value = QDir::homePath ();
	}
	else if (type == "radio" ||
			type == "combobox")
	{
		if (value.isNull () ||
				value.toString ().isEmpty ())
		{
			if (item.hasAttribute ("default"))
				value = item.attribute ("default");
			else
			{
				QDomElement option = item.firstChildElement ("option");
				while (!option.isNull ())
				{
					if (option.attribute ("default") == "true")
					{
						value = option.attribute ("name");
						break;
					}
					option = option.nextSiblingElement ("option");
				}
			}
		}
	}
	else if (type == "font")
	{
		if (value.isNull () ||
				!value.canConvert<QFont> ())
			value = QApplication::font ();
	}
	else if (type == "color")
	{
		if (!value.canConvert<QColor> ()
				|| !value.value<QColor> ().isValid ())
			value = QColor (item.attribute ("default"));
	}
	else if (type == "pushbutton") ;
	else if (type == "customwidget") ;
	else
		qWarning () << Q_FUNC_INFO << "unhandled type" << type;

	return value;
}

void XmlSettingsDialog::DoLineedit (const QDomElement& item, QFormLayout *lay)
{
	QLabel *label = new QLabel (GetLabel (item));
	label->setWordWrap (false);

	QVariant value = GetValue (item);

	QLineEdit *edit = new QLineEdit (value.toString ());
	edit->setObjectName (item.attribute ("property"));
	edit->setMinimumWidth (QApplication::fontMetrics ().width ("thisismaybeadefaultsetting"));
	if (item.hasAttribute ("password"))
		edit->setEchoMode (QLineEdit::Password);
	if (item.hasAttribute ("inputMask"))
		edit->setInputMask (item.attribute ("inputMask"));
	connect (edit,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (label, edit);
}

void XmlSettingsDialog::DoCheckbox (const QDomElement& item, QFormLayout *lay)
{
	QCheckBox *box = new QCheckBox (GetLabel (item));
	box->setObjectName (item.attribute ("property"));

	QVariant value = GetValue (item);

	box->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
	connect (box,
			SIGNAL (stateChanged (int)),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (box);
}

void XmlSettingsDialog::DoSpinbox (const QDomElement& item, QFormLayout *lay)
{
	QLabel *label = new QLabel (GetLabel (item));
	label->setWordWrap (false);
	QSpinBox *box = new QSpinBox;
	box->setObjectName (item.attribute ("property"));
	if (item.hasAttribute ("minimum"))
		box->setMinimum (item.attribute ("minimum").toInt ());
	if (item.hasAttribute ("maximum"))
		box->setMaximum (item.attribute ("maximum").toInt ());
	if (item.hasAttribute ("step"))
		box->setSingleStep (item.attribute ("step").toInt ());
	if (item.hasAttribute ("suffix"))
		box->setSuffix (item.attribute ("suffix"));
	LangElements langs = GetLangElements (item);
	if (langs.Valid_)
	{
		if (langs.Label_.first)
			label->setText (langs.Label_.second);
		if (langs.Suffix_.first)
			box->setSuffix (langs.Suffix_.second);
	}

	QVariant value = GetValue (item);

	box->setValue (value.toInt ());
	connect (box,
			SIGNAL (valueChanged (int)),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (label, box);
}

void XmlSettingsDialog::DoDoubleSpinbox (const QDomElement& item, QFormLayout *lay)
{
	QLabel *label = new QLabel (GetLabel (item));
	label->setWordWrap (false);
	QDoubleSpinBox *box = new QDoubleSpinBox;
	box->setObjectName (item.attribute ("property"));
	if (item.hasAttribute ("minimum"))
		box->setMinimum (item.attribute ("minimum").toDouble ());
	if (item.hasAttribute ("maximum"))
		box->setMaximum (item.attribute ("maximum").toDouble ());
	if (item.hasAttribute ("step"))
		box->setSingleStep (item.attribute ("step").toDouble ());
	if (item.hasAttribute ("suffix"))
		box->setSuffix (item.attribute ("suffix"));
	if (item.hasAttribute ("precision"))
		box->setDecimals (item.attribute ("precision").toInt ());
	LangElements langs = GetLangElements (item);
	if (langs.Valid_)
	{
		if (langs.Label_.first)
			label->setText (langs.Label_.second);
		if (langs.Suffix_.first)
			box->setSuffix (langs.Suffix_.second);
	}

	QVariant value = GetValue (item);

	box->setValue (value.toDouble ());
	connect (box,
			SIGNAL (valueChanged (double)),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (label, box);
}

void XmlSettingsDialog::DoGroupbox (const QDomElement& item, QFormLayout *lay)
{
	QGroupBox *box = new QGroupBox (GetLabel (item));
	box->setObjectName (item.attribute ("property"));
	QFormLayout *groupLayout = new QFormLayout ();
	groupLayout->setRowWrapPolicy (QFormLayout::DontWrapRows);
	groupLayout->setFieldGrowthPolicy (QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->setContentsMargins (2, 2, 2, 2);
	box->setLayout (groupLayout);
	box->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	box->setCheckable (true);

	QVariant value = GetValue (item);

	box->setChecked (value.toBool ());
	connect (box,
			SIGNAL (toggled (bool)),
			this,
			SLOT (updatePreferences ()));
	ParseEntity (item, box);
	
	lay->addRow (box);
}

void XmlSettingsDialog::DoSpinboxRange (const QDomElement& item, QFormLayout *lay)
{

	QLabel *label = new QLabel (GetLabel (item));
	label->setWordWrap (false);
	RangeWidget *widget = new RangeWidget ();
	widget->setObjectName (item.attribute ("property"));
	widget->SetMinimum (item.attribute ("minimum").toInt ());
	widget->SetMaximum (item.attribute ("maximum").toInt ());

	QVariant value = GetValue (item);

	widget->SetRange (value);
	connect (widget,
			SIGNAL (changed ()),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (label, widget);
}

void XmlSettingsDialog::DoPath (const QDomElement& item, QFormLayout *lay)
{
	QLabel *label = new QLabel (GetLabel (item));
	label->setWordWrap (false);

	FilePicker::Type type = FilePicker::TExistingDirectory;
	if (item.attribute ("pickerType") == "openFileName")
		type = FilePicker::TOpenFileName;
	else if (item.attribute ("pickerType") == "saveFileName")
		type = FilePicker::TSaveFileName;

	FilePicker *picker = new FilePicker (type, this);
	QVariant value = GetValue (item);
	picker->SetText (value.toString ());
	picker->setObjectName (item.attribute ("property"));
	if (item.attribute ("onCancel") == "clear")
		picker->SetClearOnCancel (true);
	if (item.hasAttribute ("filter"))
		picker->SetFilter (item.attribute ("filter"));

	connect (picker,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (label, picker);
}

void XmlSettingsDialog::DoRadio (const QDomElement& item, QFormLayout *lay)
{
	RadioGroup *group = new RadioGroup (this);
	group->setObjectName (item.attribute ("property"));

	QDomElement option = item.firstChildElement ("option");
	while (!option.isNull ())
	{
		QRadioButton *button = new QRadioButton (GetLabel (option));
		button->setObjectName (option.attribute ("name"));
		group->AddButton (button,
				option.hasAttribute ("default") &&
				option.attribute ("default") == "true");
		option = option.nextSiblingElement ("option");
	}

	QVariant value = GetValue (item);

	connect (group,
			SIGNAL (valueChanged ()),
			this,
			SLOT (updatePreferences ()));

	QGroupBox *box = new QGroupBox (GetLabel (item));
	QVBoxLayout *layout = new QVBoxLayout ();
	box->setLayout (layout);
	layout->addWidget (group);

	lay->addRow (box);
}

void XmlSettingsDialog::DoCombobox (const QDomElement& item, QFormLayout *lay)
{
	QComboBox *box = new QComboBox (this);
	box->setObjectName (item.attribute ("property"));
	if (item.hasAttribute ("maxVisibleItems"))
		box->setMaxVisibleItems (item.attribute ("maxVisibleItems").toInt ());

	QDomElement option = item.firstChildElement ("option");
	while (!option.isNull ())
	{
		QList<QImage> images = GetImages (option);
		if (images.size ())
		{
			QIcon icon = QIcon (QPixmap::fromImage (images.at (0)));
			box->addItem (icon, GetLabel (option), option.attribute ("name"));
		}
		else
			box->addItem (GetLabel (option), option.attribute ("name"));

		option = option.nextSiblingElement ("option");
	}

	connect (box,
			SIGNAL (currentIndexChanged (int)),
			this,
			SLOT (updatePreferences ()));

	QDomElement scriptContainer = item.firstChildElement ("scripts");
	if (!scriptContainer.isNull ())
	{
		Scripter scripter (scriptContainer);

		QStringList fromScript = scripter.GetOptions ();
		for (QStringList::const_iterator i = fromScript.begin (),
				end = fromScript.end (); i != end; ++i)
			box->addItem (scripter.HumanReadableOption (*i),
					*i);
	}

	int pos = box->findData (GetValue (item));
	if (pos != -1)
		box->setCurrentIndex (pos);
	else
		qWarning () << Q_FUNC_INFO
			<< box
			<< GetValue (item)
			<< "not found";

	QLabel *label = new QLabel (GetLabel (item));
	label->setWordWrap (false);

	lay->addRow (label, box);
}

void XmlSettingsDialog::DoFont (const QDomElement& item, QFormLayout *lay)
{
	QString labelString = GetLabel (item);
	QLabel *label = new QLabel (labelString);
	label->setWordWrap (false);

	FontPicker *picker = new FontPicker (labelString, this);
	picker->setObjectName (item.attribute ("property"));
	picker->SetCurrentFont (GetValue (item).value<QFont> ());

	connect (picker,
			SIGNAL (currentFontChanged (const QFont&)),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (label, picker);
}

void XmlSettingsDialog::DoColor (const QDomElement& item, QFormLayout *lay)
{
	QString labelString = GetLabel (item);
	QLabel *label = new QLabel (labelString);
	label->setWordWrap (false);

	ColorPicker *picker = new ColorPicker (labelString, this);
	picker->setObjectName (item.attribute ("property"));
	picker->SetCurrentColor (GetValue (item).value<QColor> ());

	connect (picker,
			SIGNAL (currentColorChanged (const QColor&)),
			this,
			SLOT (updatePreferences ()));

	lay->addRow (label, picker);
}

void XmlSettingsDialog::DoPushButton (const QDomElement& item, QFormLayout *lay)
{
	QPushButton *button = new QPushButton (this);
	button->setObjectName (item.attribute ("name"));
	button->setText (GetLabel (item));
	lay->addRow (button);
	connect (button,
			SIGNAL (released ()),
			this,
			SLOT (handlePushButtonReleased ()));
}

void XmlSettingsDialog::DoCustomWidget (const QDomElement& item, QFormLayout *lay)
{
	QWidget *widget = new QWidget (this);
	widget->setObjectName (item.attribute ("name"));
	QVBoxLayout *layout = new QVBoxLayout ();
	layout->setContentsMargins (0, 0, 0, 0);
	widget->setLayout (layout);
	widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	if (item.attribute ("label") == "own")
		lay->addRow (widget);
	else
	{
		QLabel *label = new QLabel (GetLabel (item));
		label->setWordWrap (false);

		lay->addRow (label, widget);
	}
}

QList<QImage> XmlSettingsDialog::GetImages (const QDomElement& item) const
{
	QList<QImage> result;
	QDomElement binary = item.firstChildElement ("binary");
	while (!binary.isNull ())
	{
		QByteArray data;
		if (binary.attribute ("place") == "rcc")
		{
			QFile file (binary.text ());
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
					<< "could not open file"
					<< binary.text ()
					<< ", because"
					<< file.errorString ();

				binary = binary.nextSiblingElement ("binary");

				continue;
			}
			data = file.readAll ();
		}
		else
		{
			QByteArray base64 = binary.text ().toLatin1 ();
			data = QByteArray::fromBase64 (base64);
		}
		if (binary.attribute ("type") == "image")
		{
			QImage image = QImage::fromData (data);
			if (!image.isNull ())
				result << image;
		}
		binary = binary.nextSiblingElement ("binary");
	}
	return result;
}

void XmlSettingsDialog::UpdateXml (bool whole)
{
	QDomNodeList nodes = Document_->elementsByTagName ("item");
	if (whole)
		for (int i = 0; i < nodes.size (); ++i)
		{
			QDomElement elem = nodes.at (i).toElement ();
			if (!elem.hasAttribute ("property"))
				continue;

			QString name = elem.attribute ("property");
			QVariant value = WorkingObject_->property (name.toLatin1 ().constData ());

			UpdateSingle (name, value, elem);
		}
	else
		for (Property2Value_t::const_iterator i = Prop2NewValue_.begin (),
				end = Prop2NewValue_.end (); i != end; ++i)
		{
			QDomElement element;
			QString name = i.key ();
			for (int j = 0, size = nodes.size (); j < size; ++j)
			{
				QDomElement e = nodes.at (j).toElement ();
				if (e.isNull ())
					continue;
				if (e.attribute ("property") == name)
				{
					element = e;
					break;
				}
			}
			if (element.isNull ())
			{
				qWarning () << Q_FUNC_INFO << "element for property" << name << "not found";
				return;
			}

			UpdateSingle (name, i.value (), element);
		}
}

void XmlSettingsDialog::UpdateSingle (const QString& name,
		const QVariant& value, QDomElement& element)
{
	Q_UNUSED (name);

	QString type = element.attribute ("type");
	if (type == "lineedit" ||
			type == "checkbox" ||
			type == "spinbox" ||
			type == "groupbox" ||
			type == "path")
		element.setAttribute ("default", value.toString ());
	else if (type == "spinboxrange")
	{
		QStringList vals = value.toStringList ();
		if (vals.size () != 2)
		{
			qWarning () << Q_FUNC_INFO << "spinboxrange value error, not 2 elems in list";
			return;
		}
		element.setAttribute ("default", vals.at (0) + ':' + vals.at (1));
	}
	else if (type == "radio" ||
			type == "combobox" ||
			type == "doublespinbox")
	{
		QDomNodeList options = element.elementsByTagName ("option");
		for (int i = 0; i < options.size (); ++i)
			options.at (i).toElement ().removeAttribute ("default");

		for (int i = 0; i < options.size (); ++i)
		{
			QDomElement option = options.at (i).toElement ();
			QString optName = value.toString ();
			if (option.attribute ("name") == optName)
			{
				option.setAttribute ("default", "true");
				break;
			}
		}
	}
}

void XmlSettingsDialog::SetValue (QWidget *object, const QVariant& value)
{
	QLineEdit *edit = qobject_cast<QLineEdit*> (object);
	QCheckBox *checkbox = qobject_cast<QCheckBox*> (object);
	QSpinBox *spinbox = qobject_cast<QSpinBox*> (object);
	QDoubleSpinBox *doubleSpinbox = qobject_cast<QDoubleSpinBox*> (object);
	QGroupBox *groupbox = qobject_cast<QGroupBox*> (object);
	RangeWidget *rangeWidget = qobject_cast<RangeWidget*> (object);
	FilePicker *picker = qobject_cast<FilePicker*> (object);
	RadioGroup *radiogroup = qobject_cast<RadioGroup*> (object);
	QComboBox *combobox = qobject_cast<QComboBox*> (object);
	FontPicker *fontPicker = qobject_cast<FontPicker*> (object);
	ColorPicker *colorPicker = qobject_cast<ColorPicker*> (object);
	if (edit)
		edit->setText (value.toString ());
	else if (checkbox)
		checkbox->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
	else if (spinbox)
		spinbox->setValue (value.toLongLong ());
	else if (doubleSpinbox)
		doubleSpinbox->setValue (value.toDouble ());
	else if (groupbox)
		groupbox->setChecked (value.toBool ());
	else if (rangeWidget)
		rangeWidget->SetRange (value);
	else if (picker)
		picker->SetText (value.toString ());
	else if (radiogroup)
		radiogroup->SetValue (value.toString ());
	else if (combobox)
	{
		int pos = combobox->findData (value);
		if (pos != -1)
			combobox->setCurrentIndex (pos);
		else
			qWarning () << Q_FUNC_INFO
				<< combobox
				<< value
				<< "not found";
	}
	else if (fontPicker)
		fontPicker->SetCurrentFont (value.value<QFont> ());
	else if (colorPicker)
		colorPicker->SetCurrentColor (value.value<QColor> ());
	else
		qWarning () << Q_FUNC_INFO << "unhandled object" << object << "for" << value;
}

bool XmlSettingsDialog::eventFilter (QObject *obj, QEvent *event)
{
	if (event->type () == QEvent::DynamicPropertyChange)
	{
		QByteArray name = static_cast<QDynamicPropertyChangeEvent*> (event)->propertyName ();

		QWidget *widget = findChild<QWidget*> (name);
		if (widget)
			SetValue (widget, obj->property (name));

		return false;
	}
	else
		return QWidget::eventFilter (obj, event);
}

void XmlSettingsDialog::accept ()
{
	for (Property2Value_t::const_iterator i = Prop2NewValue_.begin (),
			end = Prop2NewValue_.end (); i != end; ++i)
		WorkingObject_->setProperty (i.key ().toLatin1 ().constData (), i.value ());

	UpdateXml ();

	Prop2NewValue_.clear ();

	foreach (QWidget *widget, Customs_)
		QMetaObject::invokeMethod (widget, "accept");
}

void XmlSettingsDialog::reject ()
{
	for (Property2Value_t::iterator i = Prop2NewValue_.begin (); i != Prop2NewValue_.end (); ++i)
	{
		QWidget *object = findChild<QWidget*> (i.key ());
		if (!object)
		{
			qWarning () << Q_FUNC_INFO << "could not find object for property" << i.key ();
			continue;
		}

		SetValue (object,
				WorkingObject_->property (i.key ().toLatin1 ().constData ()));
	}
	
	Prop2NewValue_.clear ();

	foreach (QWidget *widget, Customs_)
		QMetaObject::invokeMethod (widget, "reject");
}

void XmlSettingsDialog::handleCustomDestroyed ()
{
	QWidget *widget = qobject_cast<QWidget*> (sender ());
	Customs_.removeAll (widget);
}

void XmlSettingsDialog::updatePreferences ()
{
	QString propertyName = sender ()->objectName ();
	if (propertyName.isEmpty ())
	{
		qWarning () << Q_FUNC_INFO << "property name is empty for object" << sender ();
		return;
	}
	QVariant value;

	QLineEdit *edit = qobject_cast<QLineEdit*> (sender ());
	QCheckBox *checkbox = qobject_cast<QCheckBox*> (sender ());
	QSpinBox *spinbox = qobject_cast<QSpinBox*> (sender ());
	QDoubleSpinBox *doubleSpinbox = qobject_cast<QDoubleSpinBox*> (sender ());
	QGroupBox *groupbox = qobject_cast<QGroupBox*> (sender ());
	RangeWidget *rangeWidget = qobject_cast<RangeWidget*> (sender ());
	FilePicker *picker = qobject_cast<FilePicker*> (sender ());
	RadioGroup *radiogroup = qobject_cast<RadioGroup*> (sender ());
	QComboBox *combobox = qobject_cast<QComboBox*> (sender ());
	FontPicker *fontPicker = qobject_cast<FontPicker*> (sender ());
	ColorPicker *colorPicker = qobject_cast<ColorPicker*> (sender ());
	if (edit)
		value = edit->text ();
	else if (checkbox)
		value = checkbox->checkState ();
	else if (spinbox)
		value = spinbox->value ();
	else if (doubleSpinbox)
		value = doubleSpinbox->value ();
	else if (fontPicker)
		value = fontPicker->GetCurrentFont ();
	else if (colorPicker)
		value = colorPicker->GetCurrentColor ();
	else if (groupbox)
		value = groupbox->isChecked ();
	else if (rangeWidget)
		value = rangeWidget->GetRange ();
	else if (picker)
		value = picker->GetText ();
	else if (radiogroup)
		value = radiogroup->GetValue ();
	else if (combobox)
		value = combobox->itemData (combobox->currentIndex ());
	else
	{
		qWarning () << Q_FUNC_INFO << "unhandled sender" << sender ();
		return;
	}

	Prop2NewValue_ [propertyName] = value;
}

void XmlSettingsDialog::handlePushButtonReleased ()
{
	emit pushButtonClicked (sender ()->objectName ());
}

