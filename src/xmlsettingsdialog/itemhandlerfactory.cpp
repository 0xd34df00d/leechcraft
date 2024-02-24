/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerfactory.h"
#include <QFormLayout>
#include <QLabel>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include "basesettingsmanager.h"
#include "xmlsettingsdialog.h"
#include "itemhandlers/itemhandlerspinboxrange.h"
#include "itemhandlers/itemhandlerpath.h"
#include "itemhandlers/itemhandlerfont.h"
#include "itemhandlers/itemhandlercolor.h"
#include "itemhandlers/itemhandlercheckbox.h"
#include "itemhandlers/itemhandlergroupbox.h"
#include "itemhandlers/itemhandlerradio.h"
#include "itemhandlers/itemhandlercombobox.h"
#include "itemhandlers/itemhandlerlineedit.h"
#include "itemhandlers/itemhandlermultiline.h"
#include "itemhandlers/itemhandlerspinbox.h"
#include "itemhandlers/itemhandlerpushbutton.h"
#include "itemhandlers/itemhandlercustomwidget.h"
#include "itemhandlers/itemhandlerdataview.h"

namespace LC
{
	ItemHandlerFactory::PropInfo::operator bool () const
	{
		return Widget_;
	}

	ItemHandlerFactory::ItemHandlerFactory (Util::XmlSettingsDialog *xsd)
	: XSD_ { *xsd }
	, Handlers_
		{
			{ "checkbox", &HandleCheckbox },
			{ "color", &HandleColor },
			{ "combobox", &HandleCombobox },
			{ "customwidget", &HandleCustomWidget },
			{ "dataview", &HandleDataView },
			{ "font", &HandleFont },
			{ "groupbox", &HandleGroupbox },
			{ "lineedit", &HandleLineEdit },
			{ "multiline", &HandleMultiline },
			{ "path", &HandlePath },
			{ "pushbutton", &HandlePushButton },
			{ "radio", &HandleRadio },
			{ "spinbox", &HandleSpinbox },
			{ "spinboxdouble", &HandleSpinboxDouble },
			{ "spinboxrange", &HandleSpinboxRange },
		}
	{
	}

	auto ItemHandlerFactory::ExpectPropChange (const QString& propName) const
	{
		ExpectedPropChanges_ << propName;
		return Util::MakeScopeGuard ([this, propName] { ExpectedPropChanges_.remove (propName); });
	}

	namespace
	{
		void AddToLayout (const ItemRepresentation& repr, const QString& label, QFormLayout& layout)
		{
			if (label.isEmpty ())
			{
				layout.addRow (repr.Widget_);
				return;
			}

			switch (repr.LabelPosition_)
			{
			case LabelPosition::Default:
				layout.addRow (label, repr.Widget_);
				break;
			case LabelPosition::Wrap:
				layout.addRow (new QLabel { label });
				layout.addRow (repr.Widget_);
				break;
			case LabelPosition::None:
				layout.addRow (repr.Widget_);
				break;
			}
		}

		void SetSearchTerms (const ItemRepresentation& repr, const QString& label)
		{
			QStringList searchTerms;
			if (repr.SearchTerms_)
				searchTerms << *repr.SearchTerms_;
			if (!label.isEmpty ())
				searchTerms << label;
			repr.Widget_->setProperty ("SearchTerms", searchTerms);
		}

		QString GetDefault (const QDomElement& element)
		{
			if (element.hasAttribute ("def"_qs))
				return element.attribute ("def"_qs);
			if (element.hasAttribute ("default"_qs))
				return element.attribute ("default"_qs);
			if (const auto& defChild = element.firstChildElement ("default"_qs); !defChild.isNull ())
				return defChild.text ();

			return {};
		}
	}

	std::optional<QVariant> ItemHandlerFactory::Handle (const QDomElement& element, QFormLayout& baseLayout)
	{
		const auto& prop = element.attribute ("property"_qs);
		const auto& type = element.attribute ("type"_qs).toLatin1 ();
		const auto& handler = Handlers_.value (type);
		if (!handler)
		{
			qWarning () << "unhandled element of type" << type << prop;
			return {};
		}

		const auto& label = XSD_.GetLabel (element);

		const auto repr = handler ({
				.Elem_ = element,
				.Label_ = label,
				.XSD_ = XSD_,
				.Prop_ = prop,
				.Default_ = GetDefault (element),
				.MarkChanged_ = [this, prop] { MarkChanged (prop); },
			});

		auto widget = repr.Widget_;
		widget->setToolTip (XSD_.GetDescription (element));
		Prop2Info_ [prop] = PropInfo {
			.Widget_ = widget,
			.Prop_ = prop,
			.Getter_ = repr.Getter_,
			.Setter_ = repr.Setter_,
			.DefaultValue_ = repr.DefaultValue_
		};

		if (repr.DataSourceSetter_)
			Prop2DataSourceSetter_ [prop] = repr.DataSourceSetter_;
		else
			SetReprValue (prop);

		SetSearchTerms (repr, label);
		AddToLayout (repr, label, baseLayout);
		return repr.DefaultValue_;
	}

	void ItemHandlerFactory::SetValue (const QString& propName, const QVariant& value) const
	{
		if (const auto& info = Prop2Info_ [propName])
		{
			if (info.Setter_)
				info.Setter_ (value);
		}
	}

	void ItemHandlerFactory::Accept ()
	{
		const auto storage = XSD_.GetManagerObject ();
		for (const auto& [prop, val] : Util::Stlize (Prop2NewVal_))
			storage->setProperty (prop.toLatin1 ().constData (), val);
		Prop2NewVal_.clear ();

		for (auto widget : CustomWidgets_)
			QMetaObject::invokeMethod (widget, "accept");
	}

	void ItemHandlerFactory::Reject ()
	{
		const auto storage = XSD_.GetManagerObject ();
		for (const auto& [prop, _] : Util::Stlize (Prop2NewVal_))
		{
			const auto& info = Prop2Info_ [prop];
			const auto guard = ExpectPropChange (prop);
			info.Setter_ (storage->property (prop.toLatin1 ().constData ()));
		}

		Prop2NewVal_.clear ();

		for (const auto widget : CustomWidgets_)
			QMetaObject::invokeMethod (widget, "reject");
	}

	void ItemHandlerFactory::SetDataSource (const QString& prop, QAbstractItemModel *model)
	{
		if (const auto setter = Prop2DataSourceSetter_.value (prop))
		{
			setter (*model);
			SetReprValue (prop);
		}
		else
			qWarning () << "there is no registered datasource setter for" << prop
					<< "; registered datasources:" << Prop2DataSourceSetter_.keys ();
	}

	void ItemHandlerFactory::SetCustomWidget (const QString& name, QWidget *widget)
	{
		const auto& container = Prop2Info_.value (name).Widget_;
		if (!container)
		{
			qWarning () << "unknown custom widget" << name << widget;
			return;
		}

		container->layout ()->addWidget (widget);
		CustomWidgets_ << widget;
		connect (widget,
				&QWidget::destroyed,
				this,
				[this, widget] { CustomWidgets_.removeOne (widget); });
	}

	void ItemHandlerFactory::SetReprValue (const QString& prop) const
	{
		const auto& info = Prop2Info_ [prop];
		if (!info)
		{
			qWarning () << "no info for" << prop;
			return;
		}

		if (!info.Setter_)
			return;

		const auto expectGuard = ExpectPropChange (prop);
		if (const auto& stored = XSD_.GetStoredValue (prop);
			!stored.isNull ())
			info.Setter_ (stored);
		else
			info.Setter_ (info.DefaultValue_);
	}

	void ItemHandlerFactory::MarkChanged (const QString& propName)
	{
		const auto& info = Prop2Info_.value (propName);
		if (!info)
		{
			qWarning () << "unknown prop" << propName;
			return;
		}

		const auto& value = info.Getter_ ();
		XSD_.GetManagerObject ()->OptionSelected (info.Prop_.toLatin1 (), value);
		if (!ExpectedPropChanges_.contains (propName))
			Prop2NewVal_ [info.Prop_] = value;
	}
}
