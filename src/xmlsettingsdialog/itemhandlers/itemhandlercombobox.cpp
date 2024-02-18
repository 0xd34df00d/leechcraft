/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlercombobox.h"
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QtDebug>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include <util/sys/resourceloader.h>
#include "../scripter.h"
#include "../xmlsettingsdialog.h"

namespace LC
{
	namespace
	{
		QVector<QImage> GetImages (const QDomElement& item)
		{
			QVector<QImage> result;

			Util::ResourceLoader loader { {} };
			loader.AddGlobalPrefix ();
			loader.AddLocalPrefix ();

			for (const auto& binary : Util::DomChildren (item, "binary"_qs))
			{
				if (binary.attribute ("type"_qs) != "image"_ql)
					continue;

				const auto& place = binary.attribute ("place"_qs);

				QImage image;
				if (place == "rcc"_ql)
					image.load (binary.text ());
				else if (place == "share"_ql)
					image = loader.LoadPixmap (binary.text ()).toImage ();
				else
					image = QImage::fromData (QByteArray::fromBase64 (binary.text ().toLatin1 ()));

				if (!image.isNull ())
					result << image;
			}
			return result;
		}

		QVariant PopulateOptions (QComboBox& box, const QDomElement& item, const Util::XmlSettingsDialog& xsd)
		{
			QVariant defValue;

			for (const auto& option : Util::DomChildren (item, "option"_qs))
			{
				const auto& name = option.attribute ("name"_qs);
				auto label = xsd.GetLabel (option);
				if (label.isEmpty ())
					label = name;

				if (defValue.isNull () || option.attribute ("default"_qs) == "true"_ql)
					defValue = name;

				if (const auto& images = GetImages (option);
					!images.isEmpty ())
					box.addItem (QPixmap::fromImage (images.at (0)), label, name);
				else
					box.addItem (label, name);

				auto setColor = [&] (const QString& attr, Qt::ItemDataRole role)
				{
					if (option.hasAttribute (attr))
					{
						const QColor color (option.attribute (attr));
						box.setItemData (box.count () - 1, color, role);
					}
				};
				setColor ("color"_qs, Qt::ForegroundRole);
				setColor ("bgcolor"_qs, Qt::BackgroundRole);
			}

			if (auto scriptContainer = item.firstChildElement ("scripts"_qs);
					!scriptContainer.isNull ())
			{
				Scripter scripter { scriptContainer };
				for (const auto& elm : scripter.GetOptions ())
					box.addItem (scripter.HumanReadableOption (elm), elm);
			}

			return defValue;
		}

		QVariant GetComboboxValue (QComboBox *box)
		{
			auto result = box->currentData ();
			if (result.isNull ())
				result = box->currentText ();
			return result;
		}

		void SetComboboxValue (QComboBox *box, const QVariant& value)
		{
			int pos = box->findData (value);
			if (pos == -1)
				pos = box->findText (value.toString ());

			if (pos != -1)
				box->setCurrentIndex (pos);
			else
				qWarning () << value << "not found";
		}

		QWidget* HandleMoreThisStuff (QComboBox *box, const QDomElement& item, Util::XmlSettingsDialog& xsd)
		{
			const auto& name = item.attribute ("moreThisStuff"_qs);
			if (name.isEmpty ())
				return box;

			const auto hbox = new QHBoxLayout;
			hbox->addWidget (box);

			const auto moreButt = new QPushButton (QObject::tr ("Get more..."));
			QObject::connect (moreButt,
					&QPushButton::released,
					&xsd,
					[&xsd, name] { emit xsd.moreThisStuffRequested (name); });
			hbox->addWidget (moreButt);

			const auto combined = new QWidget;
			combined->setLayout (hbox);
			return combined;
		}
	}

	ItemRepresentation HandleCombobox (const ItemContext& ctx)
	{
		const auto& item = ctx.Elem_;

		const auto box = new QComboBox;
		box->setSizeAdjustPolicy (QComboBox::AdjustToContents);
		if (item.hasAttribute ("maxVisibleItems"_qs))
			box->setMaxVisibleItems (item.attribute ("maxVisibleItems"_qs).toInt ());

		auto defValue = PopulateOptions (*box, item, ctx.XSD_);

		SetChangedSignal (ctx, box, &QComboBox::currentIndexChanged);

		const auto dataSourceSetter = item.attribute ("mayHaveDataSource"_qs).toLower () == "true"_ql ?
				[box] (QAbstractItemModel& m) { box->setModel (&m); } :
				DataSourceSetter {};

		return
		{
			.Widget_ = HandleMoreThisStuff (box, item, ctx.XSD_),
			.Label_ = ctx.Label_,

			.DefaultValue_ = defValue,
			.Getter_ = [box] { return GetComboboxValue (box); },
			.Setter_ = [box] (const QVariant& value) { SetComboboxValue (box, value); },

			.DataSourceSetter_ = dataSourceSetter,
		};
	}
}
