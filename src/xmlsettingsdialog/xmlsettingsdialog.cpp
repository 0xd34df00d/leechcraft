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
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <QUrl>
#include <QScrollArea>
#include <QDomNodeList>
#include <QtScript>
#include <plugininterface/util.h>
#include "itemhandlerfactory.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

XmlSettingsDialog::XmlSettingsDialog ()
: Document_ (new QDomDocument)
, HandlersManager_ (new ItemHandlerFactory ())
{
	ItemHandlerBase::SetXmlSettingsDialog (this);
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
	Basename_ = QFileInfo (basename).baseName ();
	WorkingObject_ = obj;
	QString filename;
	if (QFile::exists (basename))
		filename = basename;
	else if (QFile::exists (QString (":/") + basename))
		filename = QString (":/") + basename;
#ifdef Q_WS_WIN
	else if (QFile::exists (QString ("settings/") + basename))
		filename = QString ("settings/") + basename;
#elif defined (Q_WS_MAC)
	else if (QFile::exists (QApplication::applicationDirPath () +
			"/../Resources/settings/" + basename))
		filename = QApplication::applicationDirPath () +
				"/../Resources/settings/" + basename;
#elif defined (INSTALL_PREFIX)
	else if (QFile::exists (QString (INSTALL_PREFIX "/share/leechcraft/settings/") + basename))
		filename = QString (INSTALL_PREFIX "/share/leechcraft/settings/") + basename;
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

	QString emsg;
	int eline;
	int ecol;
	if (!Document_->setContent (data, &emsg, &eline, &ecol))
	{
		qWarning () << "Could not parse file, line"
			<< eline
			<< "; column"
			<< ecol
			<< emsg;
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

		QWidget *object = findChild<QWidget*> (propName);
		if (!object)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not find object for property"
				<< propName;
			continue;
		}
		HandlersManager_->SetValue (object, value);
	}

	UpdateXml ();
	HandlersManager_->ClearNewValues ();
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

void XmlSettingsDialog::SetDataSource (const QString& property,
		QAbstractItemModel *dataSource)
{
	HandlersManager_->SetDataSource (property, dataSource, this);
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
	QGridLayout *lay = new QGridLayout;
	lay->setContentsMargins (0, 0, 0, 0);
	baseWidget->setLayout (lay);
	baseWidget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	ParseEntity (page, baseWidget);
	QSpacerItem *verticalSpacer = new QSpacerItem (10, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);
	lay->addItem (verticalSpacer, lay->rowCount (), 0);
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
		QGridLayout *groupLayout = new QGridLayout ();
		groupLayout->setContentsMargins (2, 2, 2, 2);
		box->setLayout (groupLayout);
		box->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
		ParseEntity (gbox, box);

		QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
		lay->addWidget (box, lay->rowCount (), 0);

		gbox = gbox.nextSiblingElement ("groupbox");

		QSpacerItem *verticalSpacer = new QSpacerItem (10, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
		groupLayout->addItem (verticalSpacer, groupLayout->rowCount (), 0);
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
		QGridLayout *areaLayout = new QGridLayout;
		areaWidget->setLayout (areaLayout);
		ParseEntity (scroll, areaWidget);
		area->setWidget (areaWidget);
		areaWidget->show ();

		QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
		lay->addWidget (area, lay->rowCount (), 0, 1, 2);

		scroll = scroll.nextSiblingElement ("scrollarea");
		QSpacerItem *verticalSpacer = new QSpacerItem (10, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
		lay->addItem (verticalSpacer, lay->rowCount (), 0, 1, 1);
	}

	QDomElement tab = entity.firstChildElement ("tab");
	if (!tab.isNull ())
	{
		QTabWidget *tabs = new QTabWidget;
		QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
		lay->addWidget (tabs, lay->rowCount (), 0, 1, 2);
		while (!tab.isNull ())
		{
			QWidget *page = new QWidget;
			QGridLayout *widgetLay = new QGridLayout;
			widgetLay->setContentsMargins (0, 0, 0, 0);
			page->setLayout (widgetLay);
			page->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
			tabs->addTab (page, GetLabel (tab));
			ParseEntity (tab, page);
			tab = tab.nextSiblingElement ("tab");
			QSpacerItem *verticalSpacer = new QSpacerItem (10, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
			widgetLay->addItem (verticalSpacer, widgetLay->rowCount (), 0, 1, 1);
		}
	}
}

void XmlSettingsDialog::ParseItem (const QDomElement& item, QWidget *baseWidget)
{
	QString type = item.attribute ("type");

	QString property = item.attribute ("property");

	if (type.isEmpty () || type.isNull ())
		return;

	if (!HandlersManager_->Handle (item, baseWidget))
		qWarning () << Q_FUNC_INFO << "unhandled type" << type;

	WorkingObject_->setProperty (property.toLatin1 ().constData (), GetValue (item));
}

#if defined (Q_WS_WIN) || defined (Q_WS_MAC)
#include <QCoreApplication>
#include <QLocale>

namespace
{
	QString GetLanguageHack ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
		QString localeName = settings.value ("Language", "system").toString ();

		if (localeName == "system")
		{
			localeName = QString (::getenv ("LANG")).left (5);
			if (localeName.isEmpty () || localeName.size () != 5)
				localeName = QLocale::system ().name ();
		}

		return localeName.left (2);
	}
};

#endif

QString XmlSettingsDialog::GetLabel (const QDomElement& item) const
{
	QString result = "<no label>";
	QDomElement label = item.firstChildElement ("label");
	if (!label.isNull ())
		result = label.attribute ("value");
	return QCoreApplication::translate (qPrintable (Basename_),
			result.toUtf8 ().constData ());
}

QString XmlSettingsDialog::GetDescription (const QDomElement& item) const
{
	QDomElement label = item.firstChildElement ("descr");
	if (!label.isNull ())
		return label.text ();
	return QString ();
}

void XmlSettingsDialog::SetTooltip (QWidget *widget, const QDomElement& from) const
{
	QString descr = GetDescription (from);
	if (!descr.isEmpty ())
		widget->setToolTip (descr);
}

XmlSettingsDialog::LangElements XmlSettingsDialog::GetLangElements (const QDomElement& parent) const
{
	LangElements returning;
	returning.Valid_ = true;

	QDomElement label = parent.firstChildElement ("label");
	if (!label.isNull ())
	{
		returning.Label_.first = true;
		returning.Label_.second = QCoreApplication::translate (qPrintable (Basename_),
				label.attribute ("value").toUtf8 ().constData ());
	}

	QDomElement suffix = parent.firstChildElement ("suffix");
	if (!suffix.isNull ())
	{
		returning.Suffix_.first = true;
		returning.Suffix_.second = QCoreApplication::translate (qPrintable (Basename_),
				suffix.attribute ("value").toUtf8 ().constData ());
	}
	return returning;
}

QString XmlSettingsDialog::GetBasename () const
{
	return Basename_;
}

QVariant XmlSettingsDialog::GetValue (const QDomElement& item, bool ignoreObject) const
{
	QString property = item.attribute ("property");

	QVariant value;
	if (ignoreObject)
	{
		QString def = item.attribute ("default");
		if (item.attribute ("translatable") == "true")
			def = QCoreApplication::translate (qPrintable (Basename_),
					def.toUtf8 ().constData ());
		value = def;
	}
	else
	{
		QVariant tmpValue = WorkingObject_->property (property.toLatin1 ().constData ());
		if (tmpValue.isValid ())
			return tmpValue;
	}

	return HandlersManager_->GetValue (item, value);
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
	{
		const ItemHandlerBase::Prop2NewValue_t& props =
				HandlersManager_->GetNewValues ();
		for (ItemHandlerBase::Prop2NewValue_t::const_iterator i = props.begin (),
				end = props.end (); i != end; ++i)
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
}

void XmlSettingsDialog::UpdateSingle (const QString&,
		const QVariant& value, QDomElement& element)
{
	HandlersManager_->UpdateSingle (element, value);
}

void XmlSettingsDialog::SetValue (QWidget *object, const QVariant& value)
{
	HandlersManager_->SetValue (object, value);
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
	const ItemHandlerBase::Prop2NewValue_t& props =
			HandlersManager_->GetNewValues ();
	for (ItemHandlerBase::Prop2NewValue_t::const_iterator i = props.begin (),
			end = props.end (); i != end; ++i)
		WorkingObject_->setProperty (i.key ().toLatin1 ().constData (),
				i.value ());

	UpdateXml ();

	HandlersManager_->ClearNewValues ();

	Q_FOREACH (QWidget *widget, Customs_)
		QMetaObject::invokeMethod (widget, "accept");
}

void XmlSettingsDialog::reject ()
{
	const ItemHandlerBase::Prop2NewValue_t& props =
			HandlersManager_->GetNewValues ();
	for (ItemHandlerBase::Prop2NewValue_t::const_iterator i = props.begin (),
			end = props.end (); i != end; ++i)
	{
		QWidget *object = findChild<QWidget*> (i.key ());
		if (!object)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not find object for property"
				<< i.key ();
			continue;
		}

		SetValue (object,
				WorkingObject_->property (i.key ().toLatin1 ().constData ()));
	}

	HandlersManager_->ClearNewValues ();

	Q_FOREACH (QWidget *widget, Customs_)
		QMetaObject::invokeMethod (widget, "reject");
}

void XmlSettingsDialog::handleCustomDestroyed ()
{
	QWidget *widget = qobject_cast<QWidget*> (sender ());
	Customs_.removeAll (widget);
}

void XmlSettingsDialog::handlePushButtonReleased ()
{
	emit pushButtonClicked (sender ()->objectName ());
}
