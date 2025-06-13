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
#include <QStackedWidget>
#include <QFormLayout>
#include <QApplication>
#include <QScrollArea>
#include <QDomNodeList>
#include <QTabWidget>
#include <util/util.h>
#include <util/sll/debugprinters.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include <util/sll/util.h>
#include <util/sys/resourceloader.h>
#include "itemhandlerfactory.h"
#include "basesettingsmanager.h"

namespace LC::Util
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

	XmlSettingsDialog::~XmlSettingsDialog () = default;

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
			qWarning () << "cannot open file" << filename << basename;
			return;
		}

		if (const auto result = Document_->setContent (&file);
			!result)
		{
			qWarning () << "could not parse" << filename << result;
			return;
		}

		const auto& root = Document_->documentElement ();
		if (root.tagName () != "settings")
		{
			qWarning () << "bad settings file" << filename;
			return;
		}

		{
			auto initGuard = obj->EnterInitMode ();

			for (const auto& pageChild : DomChildren (root, "page"))
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
		HandlersManager_->SetCustomWidget (name, widget);
	}

	void XmlSettingsDialog::SetDataSource (const QString& property, QAbstractItemModel *dataSource)
	{
		HandlersManager_->SetDataSource (property, dataSource);
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
		const auto lay = new QFormLayout;
		lay->setContentsMargins ({});
		baseWidget->setLayout (lay);

		ParseEntity (page, *lay);
	}

	void XmlSettingsDialog::ParseEntity (const QDomElement& entity, QFormLayout& baseLayout)
	{
		for (const auto& item : Util::DomChildren (entity, "item"))
			ParseItem (item, baseLayout);

		for (const auto& gbox : Util::DomChildren (entity, "groupbox"))
		{
			const auto box = new QGroupBox (GetLabel (gbox));
			box->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
			const auto groupLayout = new QFormLayout;
			groupLayout->setContentsMargins (2, 2, 2, 2);
			box->setLayout (groupLayout);

			ParseEntity (gbox, *groupLayout);

			baseLayout.addRow (box);
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
			const auto areaLayout = new QFormLayout;
			areaWidget->setLayout (areaLayout);
			ParseEntity (scroll, *areaLayout);
			area->setWidget (areaWidget);
			area->setWidgetResizable (true);
			areaWidget->show ();

			baseLayout.addRow (area);
		}

		if (!entity.firstChildElement ("tab").isNull ())
		{
			auto tabs = new QTabWidget;
			baseLayout.addRow (tabs);

			for (const auto& tab : Util::DomChildren (entity, "tab"))
			{
				const auto page = new QWidget;
				const auto widgetLay = new QFormLayout;
				widgetLay->setContentsMargins (0, 0, 0, 0);
				page->setLayout (widgetLay);
				tabs->addTab (page, GetLabel (tab));
				ParseEntity (tab, *widgetLay);
			}
		}
	}

	void XmlSettingsDialog::ParseItem (const QDomElement& item, QFormLayout& baseLayout)
	{
		const auto& type = item.attribute ("type");
		const auto& property = item.attribute ("property");

		if (type.isEmpty ())
		{
			qWarning () << "invalid type for prop" << property;
			return;
		}

		const auto& defaultValue = HandlersManager_->Handle (item, baseLayout);
		if (!defaultValue)
		{
			qWarning () << "unhandled type" << type;
			return;
		}

		if (GetStoredValue (property).isNull ())
			WorkingObject_->setProperty (property.toLatin1 ().constData (), *defaultValue);
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
					value->toUtf8 ().replace ('\t', QByteArray {}).trimmed ().constData (),
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

	XmlSettingsDialog::LangElements XmlSettingsDialog::GetLangElements (const QDomElement& parent) const
	{
		return
		{
			.Label_ = GetUserText (parent, "label", TrContext_),
			.Suffix_ = GetUserText (parent, "suffix", TrContext_),
			.SpecialValue_ = GetUserText (parent, "special", TrContext_),
		};
	}

	QByteArray XmlSettingsDialog::GetTrContext () const
	{
		return TrContext_;
	}

	QVariant XmlSettingsDialog::GetStoredValue (const QString& property) const
	{
		return WorkingObject_->property (property.toLatin1 ().constData ());
	}

	bool XmlSettingsDialog::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () == QEvent::DynamicPropertyChange)
		{
			const auto& name = static_cast<QDynamicPropertyChangeEvent*> (event)->propertyName ();
			HandlersManager_->SetValue (name, obj->property (name));
			return false;
		}
		return QObject::eventFilter (obj, event);
	}

	void XmlSettingsDialog::accept ()
	{
		HandlersManager_->Accept ();
	}

	void XmlSettingsDialog::reject ()
	{
		HandlersManager_->Reject ();
	}

	void XmlSettingsDialog::handleShowPageRequested (BaseSettingsManager *bsm, const QString& name)
	{
		emit showPageRequested (bsm, name);

		if (name.isEmpty ())
			return;

		auto child = findChild<QWidget*> (name);
		if (!child)
		{
			qWarning () << Basename_ << "cannot find child" << name;
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
