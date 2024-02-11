/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlsettingsdialog.h"
#include <stdexcept>
#include <QFile>
#include <QtDebug>
#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QGridLayout>
#include <QApplication>
#include <QUrl>
#include <QScrollArea>
#include <QComboBox>
#include <QTextDocument>
#include <QDomNodeList>
#include <QtScript>
#include <util/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <util/sll/prelude.h>
#include <util/sll/domchildrenrange.h>
#include <util/sys/resourceloader.h>
#include "itemhandlerfactory.h"
#include "basesettingsmanager.h"

namespace LC
{
namespace Util
{
	XmlSettingsDialog::XmlSettingsDialog ()
	: Widget_ { new QWidget }
	, Pages_ { new QStackedWidget { Widget_ } }
	, Document_ { std::make_shared<QDomDocument> () }
	, HandlersManager_ { new ItemHandlerFactory { this } }
	{
		const auto mainLay = new QHBoxLayout (Widget_);
		mainLay->setContentsMargins (0, 0, 0, 0);
		mainLay->addWidget (Pages_);
		Widget_->setLayout (mainLay);
	}

	XmlSettingsDialog::~XmlSettingsDialog ()
	{
	}

	void XmlSettingsDialog::RegisterObject (BaseSettingsManager *obj, const QString& basename)
	{
		Basename_ = QFileInfo (basename).baseName ();
		TrContext_ = basename.endsWith (".xml") ?
				Basename_.toUtf8 () :
				QFileInfo { basename }.fileName ().toUtf8 ();
		WorkingObject_ = obj;
		QString filename;
		if (QFile::exists (basename))
			filename = basename;
		else if (QFile::exists (QString (":/") + basename))
			filename = QString (":/") + basename;
	#ifdef Q_OS_WIN32
		else if (QFile::exists (QApplication::applicationDirPath () + "/settings/" + basename))
			filename = QApplication::applicationDirPath () + "/settings/" + basename;
	#elif defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
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
		const QByteArray& data = file.readAll ();
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
		const QDomElement& root = Document_->documentElement ();
		if (root.tagName () != "settings")
		{
			qWarning () << "Bad settings file";
			return;
		}

		{
			auto initGuard = obj->EnterInitMode ();

			for (const auto& declaration : Util::DomChildren (root, "declare"))
				HandleDeclaration (declaration);

			for (const auto& pageChild : Util::DomChildren (root, "page"))
				ParsePage (pageChild);
		}

		obj->installEventFilter (this);

		connect (obj,
				SIGNAL (showPageRequested (Util::BaseSettingsManager*, QString)),
				this,
				SLOT (handleShowPageRequested (Util::BaseSettingsManager*, QString)));
	}

	BaseSettingsManager* XmlSettingsDialog::GetManagerObject () const
	{
		return WorkingObject_;
	}

	QWidget* XmlSettingsDialog::GetWidget () const
	{
		return Widget_;
	}

	namespace
	{
		QList<QWidget*> FindDirectChildren (QWidget *widget)
		{
			auto result = widget->findChildren<QWidget*> ();
			for (auto i = result.begin (); i != result.end (); )
			{
				const auto& sub = *i;
				if (sub->parentWidget () && sub->parentWidget () != widget)
					i = result.erase (i);
				else
					++i;
			}
			return result;
		}

		void EnableChildren (QWidget *widget)
		{
			for (auto tab : widget->findChildren<QTabWidget*> ())
				for (int i = 0; i < tab->count (); ++i)
					tab->setTabEnabled (i, true);

			for (auto child : widget->findChildren<QWidget*> ())
				child->setEnabled (true);
			widget->setEnabled (true);
		}

		bool HighlightWidget (QWidget *widget, const QString& query, ItemHandlerFactory *factory)
		{
			auto allChildren = FindDirectChildren (widget);

			const auto& terms = widget->property ("SearchTerms").toStringList ();
			if (std::any_of (terms.begin (), terms.end (),
					[&query] (const auto& term) { return term.contains (query, Qt::CaseInsensitive); }))
			{
				EnableChildren (widget);
				return true;
			}

			bool result = false;

			for (auto tab : widget->findChildren<QTabWidget*> ())
				for (int i = 0; i < tab->count (); ++i)
				{
					const bool tabTextMatches = tab->tabText (i).contains (query, Qt::CaseInsensitive);
					const bool tabMatches = tabTextMatches ||
							HighlightWidget (tab->widget (i), query, factory);
					tab->setTabEnabled (i, tabMatches);
					if (tabTextMatches)
						EnableChildren (tab->widget (i));

					for (auto tabChild : tab->findChildren<QWidget*> ())
						allChildren.removeAll (tabChild);

					if (tabMatches)
					{
						tab->setEnabled (true);
						result = true;
					}
				}

			for (auto child : allChildren)
				if (HighlightWidget (child, query, factory))
					result = true;

			widget->setEnabled (result);

			return result;
		}
	}

	QList<int> XmlSettingsDialog::HighlightMatches (const QString& query)
	{
		QList<int> result;
		if (query.isEmpty ())
		{
			for (int i = 0; i < Pages_->count (); ++i)
			{
				EnableChildren (Pages_->widget (i));
				result << i;
			}
			return result;
		}

		for (int i = 0; i < Pages_->count (); ++i)
		{
			if (Titles_.at (i).contains (query, Qt::CaseInsensitive))
			{
				EnableChildren (Pages_->widget (i));
				result << i;
				continue;
			}

			if (HighlightWidget (Pages_->widget (i), query, HandlersManager_))
				result << i;
		}

		return result;
	}

	void XmlSettingsDialog::SetCustomWidget (const QString& name, QWidget *widget)
	{
		const auto& widgets = Widget_->findChildren<QWidget*> (name);
		if (!widgets.size ())
			throw std::runtime_error (qPrintable (QString ("Widget %1 not "
							"found").arg (name)));
		if (widgets.size () > 1)
			throw std::runtime_error (qPrintable (QString ("Widget %1 "
							"appears to exist more than once").arg (name)));

		widgets.at (0)->layout ()->addWidget (widget);
		Customs_ << widget;
		connect (widget,
				&QWidget::destroyed,
				this,
				[this, widget] { Customs_.removeAll (widget); });
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

	QIcon XmlSettingsDialog::GetPageIcon (int page) const
	{
		for (const auto& name : IconNames_.value (page))
		{
			const QString themeMarker { "theme://" };
			if (name.startsWith (themeMarker))
				return QIcon::fromTheme (name.mid (themeMarker.size ()));

			const QIcon icon { name };
			if (!icon.isNull ())
				return icon;
		}

		return {};
	}

	void XmlSettingsDialog::HandleDeclaration (const QDomElement& decl)
	{
		if (decl.hasAttribute ("defaultlang"))
			DefaultLang_ = decl.attribute ("defaultlang");
	}

	void XmlSettingsDialog::ParsePage (const QDomElement& page)
	{
		Titles_ << GetLabel (page);

		QStringList icons;
		if (page.hasAttribute ("icon"))
			icons << page.attribute ("icon");

		icons += Util::Map (Util::DomChildren (page.firstChildElement ("icons"), "icon"), &QDomElement::text);
		IconNames_ << icons;

		const auto baseWidget = new QWidget;
		Pages_->addWidget (baseWidget);
		const auto lay = new QGridLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		baseWidget->setLayout (lay);

		ParseEntity (page, baseWidget);

		bool foundExpanding = false;

		for (const auto w : baseWidget->findChildren<QWidget*> ())
			if (w->sizePolicy ().verticalPolicy () & QSizePolicy::ExpandFlag)
			{
				foundExpanding = true;
				break;
			}

		if (!foundExpanding)
			lay->addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
					lay->rowCount (), 0, 1, 2);
	}

	void XmlSettingsDialog::ParseEntity (const QDomElement& entity, QWidget *baseWidget)
	{
		for (const auto& item : Util::DomChildren (entity, "item"))
			ParseItem (item, baseWidget);

		for (const auto& gbox : Util::DomChildren (entity, "groupbox"))
		{
			const auto box = new QGroupBox (GetLabel (gbox));
			box->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
			const auto groupLayout = new QGridLayout ();
			groupLayout->setContentsMargins (2, 2, 2, 2);
			box->setLayout (groupLayout);

			ParseEntity (gbox, box);

			const auto lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
			lay->addWidget (box, lay->rowCount (), 0, 1, 2);
		}

		for (const auto& scroll : Util::DomChildren (entity, "scrollarea"))
		{
			const auto area = new QScrollArea ();
			if (scroll.hasAttribute ("horizontalScroll"))
			{
				const auto& attr = scroll.attribute ("horizontalScroll");
				if (attr == "on")
					area->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
				else if (attr == "off")
					area->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
			}
			if (scroll.hasAttribute ("verticalScroll"))
			{
				const auto& attr = scroll.attribute ("verticalScroll");
				if (attr == "on")
					area->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
				else if (attr == "off")
					area->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
			}

			area->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

			const auto areaWidget = new QFrame;
			areaWidget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
			const auto areaLayout = new QGridLayout;
			areaWidget->setLayout (areaLayout);
			ParseEntity (scroll, areaWidget);
			area->setWidget (areaWidget);
			area->setWidgetResizable (true);
			areaWidget->show ();

			const auto lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
			const auto thisRow = lay->rowCount ();
			lay->addWidget (area, thisRow, 0, 1, 2);
			lay->setRowStretch (thisRow, 1);
		}

		QTabWidget *tabs = nullptr;

		for (const auto& tab : Util::DomChildren (entity, "tab"))
		{
			if (!tabs)
			{
				tabs = new QTabWidget;

				const auto lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
				lay->addWidget (tabs, lay->rowCount (), 0, 1, 2);
			}

			const auto page = new QWidget;
			const auto widgetLay = new QGridLayout;
			widgetLay->setContentsMargins (0, 0, 0, 0);
			page->setLayout (widgetLay);
			tabs->addTab (page, GetLabel (tab));
			ParseEntity (tab, page);

			widgetLay->addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
					widgetLay->rowCount (), 0, 1, 1);
		}
	}

	void XmlSettingsDialog::ParseItem (const QDomElement& item, QWidget *baseWidget)
	{
		const QString& type = item.attribute ("type");

		const QString& property = item.attribute ("property");

		if (type.isEmpty () || type.isNull ())
			return;

		if (!HandlersManager_->Handle (item, baseWidget))
			qWarning () << Q_FUNC_INFO << "unhandled type" << type;

		WorkingObject_->setProperty (property.toLatin1 ().constData (), GetValue (item));
	}

	namespace
	{
		std::optional<QString> GetUserTextUntranslated (const QDomElement& parent, const QString& textAttr)
		{
			if (parent.hasAttribute (textAttr))
				return parent.attribute (textAttr);

			const auto& elem = parent.firstChildElement (textAttr);
			if (elem.hasAttribute ("value"))
				return elem.attribute ("value");

			if (!elem.text ().isEmpty ())
				return elem.text ();

			return {};
		}

		std::optional<QString> GetUserText (const QDomElement& parent, const QString& textAttr, const QByteArray& trCtx)
		{
			const auto& value = GetUserTextUntranslated (parent, textAttr);
			if (!value)
				return {};

			return QCoreApplication::translate (trCtx.constData (),
					value->toUtf8 ().constData (),
					nullptr);
		}
	}

	QString XmlSettingsDialog::GetLabel (const QDomElement& item) const
	{
		return GetUserText (item, "label", TrContext_).value_or (QString {});
	}

	QString XmlSettingsDialog::GetDescription (const QDomElement& item) const
	{
		return GetUserText (item, "tooltip", TrContext_).value_or (QString {});
	}

	void XmlSettingsDialog::SetTooltip (QWidget *widget, const QDomElement& from) const
	{
		const auto& descr = GetDescription (from);
		if (!descr.isEmpty ())
			widget->setToolTip (descr);
	}

	XmlSettingsDialog::LangElements XmlSettingsDialog::GetLangElements (const QDomElement& parent) const
	{
		return
		{
			.Label_ = GetUserText (parent, "label", TrContext_),
			.Suffix_ = GetUserText (parent, "suffix", TrContext_),
			.SpecialValue_ = GetUserText (parent, "specialValue", TrContext_),
		};
	}

	QString XmlSettingsDialog::GetBasename () const
	{
		return Basename_;
	}

	QVariant XmlSettingsDialog::GetValue (const QDomElement& item) const
	{
		const auto& property = item.attribute ("property");

		const auto& tmpValue = WorkingObject_->property (property.toLatin1 ().constData ());
		if (tmpValue.isValid ())
			return tmpValue;

		return HandlersManager_->GetValue (item, {});
	}

	QList<QImage> XmlSettingsDialog::GetImages (const QDomElement& item) const
	{
		QList<QImage> result;

		Util::ResourceLoader loader { {} };
		loader.AddGlobalPrefix ();
		loader.AddLocalPrefix ();

		for (const auto& binary : Util::DomChildren (item, "binary"))
		{
			if (binary.attribute ("type") != "image")
				continue;

			QImage image;
			if (binary.attribute ("place") == "rcc")
				image.load (binary.text ());
			else if (binary.attribute ("place") == "share")
				image = loader.LoadPixmap (binary.text ()).toImage ();
			else
				image = QImage::fromData (QByteArray::fromBase64 (binary.text ().toLatin1 ()));

			if (!image.isNull ())
				result << image;
		}
		return result;
	}

	void XmlSettingsDialog::SetValue (QWidget *object, const QVariant& value)
	{
		HandlersManager_->SetValue (object, value);
	}

	bool XmlSettingsDialog::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () == QEvent::DynamicPropertyChange)
		{
			const auto& name = static_cast<QDynamicPropertyChangeEvent*> (event)->propertyName ();

			if (const auto widget = findChild<QWidget*> (name))
				SetValue (widget, obj->property (name));

			return false;
		}
		else
			return QObject::eventFilter (obj, event);
	}

	void XmlSettingsDialog::accept ()
	{
		for (const auto& pair : Util::Stlize (HandlersManager_->GetNewValues ()))
			WorkingObject_->setProperty (pair.first.toLatin1 ().constData (), pair.second);

		HandlersManager_->ClearNewValues ();

		for (auto widget : Customs_)
			QMetaObject::invokeMethod (widget, "accept");
	}

	void XmlSettingsDialog::reject ()
	{
		for (const auto& pair : Util::Stlize (HandlersManager_->GetNewValues ()))
		{
			const auto object = findChild<QWidget*> (pair.first);
			if (!object)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not find object for property"
					<< pair.first;
				continue;
			}

			SetValue (object,
					WorkingObject_->property (pair.first.toLatin1 ().constData ()));
		}

		HandlersManager_->ClearNewValues ();

		for (const auto widget : Customs_)
			QMetaObject::invokeMethod (widget, "reject");
	}

	void XmlSettingsDialog::handleMoreThisStuffRequested ()
	{
		emit moreThisStuffRequested (sender ()->objectName ());
	}

	void XmlSettingsDialog::handlePushButtonReleased ()
	{
		emit pushButtonClicked (sender ()->objectName ());
	}

	void XmlSettingsDialog::handleShowPageRequested (BaseSettingsManager *bsm, const QString& name)
	{
		emit showPageRequested (bsm, name);

		if (name.isEmpty ())
			return;

		auto child = findChild<QWidget*> (name);
		if (!child)
		{
			qWarning () << Q_FUNC_INFO
					<< Basename_
					<< "cannot find child"
					<< name;
			return;
		}

		QWidget *lastTabChild = nullptr;

		while (auto parent = child->parentWidget ())
		{
			const auto nextGuard = Util::MakeScopeGuard ([&child, parent] { child = parent; });

			const auto pgIdx = Pages_->indexOf (parent);
			if (pgIdx >= 0)
			{
				Pages_->setCurrentIndex (pgIdx);
				continue;
			}

			if (qobject_cast<QStackedWidget*> (parent))
				lastTabChild = child;
			else if (auto tw = qobject_cast<QTabWidget*> (parent))
				tw->setCurrentWidget (lastTabChild);
		}
	}
}
}
