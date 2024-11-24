/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerdataview.h"
#include <QGridLayout>
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QtDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include "../widgets/dataviewwidget.h"
#include "../widgets/filepicker.h"
#include "../widgets/colorpicker.h"
#include "../itemhandlerfactory.h"
#include "../datasourceroles.h"
#include "../xmlsettingsdialog.h"

namespace LC
{
	namespace
	{
		struct ColumnInfo
		{
			DataSources::DataFieldType Type_;
			QList<DataSources::EnumValueInfo> ValuesInfo_;
			QString Name_;
			bool IsConst_;
		};

		QList<ColumnInfo> GetColumnInfos (QAbstractItemModel *model)
		{
			using namespace DataSources;

			QList<ColumnInfo> infos;
			for (int i = 0, size = model->columnCount (); i < size; ++i)
			{
				const auto atRole = [&] (int role) { return model->headerData (i, Qt::Horizontal, role); };

				const auto& hData = atRole (DataSourceRole::FieldType);
				const auto type = static_cast<DataFieldType> (hData.value<int> ());
				if (type == DataFieldType::None)
					continue;

				const auto& name = atRole (Qt::DisplayRole).toString ();

				const auto& valuesVar = atRole (DataSourceRole::FieldValues).toList ();
				const auto& valGen = atRole (DataSourceRole::FieldValuesGenerator).value<EnumValueInfoGenerator> ();
				const auto& values = valuesVar.isEmpty () && valGen ?
						valGen () :
						Util::Map (valuesVar, &QVariant::value<EnumValueInfo>);

				const bool isConst = atRole (DataSourceRole::FieldNonModifiable).toBool ();
				infos.push_back ({ type, values, name, isConst });
			}
			return infos;
		}

		QWidget* GetEditor (DataSources::DataFieldType type, const QList<DataSources::EnumValueInfo>& valuesInfo)
		{
			switch (type)
			{
			case DataSources::DataFieldType::None:
				return nullptr;
			case DataSources::DataFieldType::Integer:
			{
				auto sb = new QSpinBox ();
				sb->setRange (-2000000, 2000000);
				return sb;
			}
			case DataSources::DataFieldType::String:
			case DataSources::DataFieldType::Url:
				return new QLineEdit ();
			case DataSources::DataFieldType::LocalPath:
				return new FilePicker (FilePicker::Type::ExistingDirectory);
			case DataSources::DataFieldType::Enum:
			{
				auto box = new QComboBox;
				for (const auto& info : valuesInfo)
					box->addItem (info.Icon_, info.Name_, info.UserData_);
				return box;
			}
			case DataSources::DataFieldType::Color:
				return new ColorPicker;
			case DataSources::DataFieldType::Font:
				return new QFontComboBox;
			}

			qWarning () << "unhandled editor type"
					<< type;
			return nullptr;
		}

		void SetData (QWidget *editor, DataSources::DataFieldType type, const QVariant& var)
		{
			switch (type)
			{
			case DataSources::DataFieldType::Integer:
				qobject_cast<QSpinBox*> (editor)->setValue (var.toInt ());
				break;
			case DataSources::DataFieldType::String:
			case DataSources::DataFieldType::Url:
				qobject_cast<QLineEdit*> (editor)->setText (var.toString ());
				break;
			case DataSources::DataFieldType::LocalPath:
				qobject_cast<FilePicker*> (editor)->SetText (var.toString ());
				break;
			case DataSources::DataFieldType::Enum:
				// TODO
				break;
			case DataSources::DataFieldType::Color:
				qobject_cast<ColorPicker*> (editor)->SetCurrentColor (var.value<QColor> ());
				break;
			case DataSources::DataFieldType::Font:
				qobject_cast<QFontComboBox*> (editor)->setCurrentFont (var.value<QFont> ());
				break;
			case DataSources::DataFieldType::None:
				break;
			}
		}

		QVariant GetData (QWidget *editor, DataSources::DataFieldType type)
		{
			switch (type)
			{
			case DataSources::DataFieldType::None:
				return {};
			case DataSources::DataFieldType::Integer:
				return qobject_cast<QSpinBox*> (editor)->value ();
			case DataSources::DataFieldType::String:
			case DataSources::DataFieldType::Url:
				return qobject_cast<QLineEdit*> (editor)->text ();
			case DataSources::DataFieldType::LocalPath:
				return qobject_cast<FilePicker*> (editor)->GetText ();
			case DataSources::DataFieldType::Enum:
			{
				auto box = qobject_cast<QComboBox*> (editor);
				return box->itemData (box->currentIndex ());
			}
			case DataSources::DataFieldType::Color:
				return qobject_cast<ColorPicker*> (editor)->GetCurrentColor ();
			case DataSources::DataFieldType::Font:
				return qobject_cast<QFontComboBox*> (editor)->currentFont ();
			}

			qWarning () << "unknown field type"
					<< type;
			return {};
		}

		QVariantList GetAddVariants (QAbstractItemModel *model, const QVariantList& existing)
		{
			const auto& infos = GetColumnInfos (model);

			QDialog dia;
			const auto lay = new QGridLayout ();
			dia.setLayout (lay);

			QList<QWidget*> dataWidgets;
			for (int i = 0; i < infos.size (); ++i)
			{
				const auto& info = infos.at (i);

				if (info.Type_ == DataSources::Enum && info.ValuesInfo_.isEmpty ())
				{
					if (existing.isEmpty ())
						return {};
					dataWidgets << nullptr;
					continue;
				}

				if (!existing.isEmpty () && info.IsConst_)
				{
					dataWidgets << nullptr;
					continue;
				}

				const auto editorWidget = GetEditor (info.Type_, info.ValuesInfo_);
				dataWidgets << editorWidget;

				SetData (editorWidget, info.Type_, existing.value (i));

				const int row = lay->rowCount ();
				lay->addWidget (new QLabel { info.Name_ }, row, 0, Qt::AlignRight);
				lay->addWidget (editorWidget, row, 1);
			}
			const auto buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
					Qt::Horizontal, &dia);
			QObject::connect (buttons,
					&QDialogButtonBox::accepted,
					&dia,
					&QDialog::accept);
			QObject::connect (buttons,
					&QDialogButtonBox::rejected,
					&dia,
					&QDialog::reject);
			lay->addWidget (buttons, lay->rowCount (), 0, 1, -1);

			if (dia.exec () != QDialog::Accepted)
				return {};

			QVariantList datas;
			for (int i = 0, size = infos.size (); i < size; ++i)
			{
				auto w = dataWidgets.value (i);
				datas << (w ? GetData (w, infos.at (i).Type_) : QVariant {});
			}
			return datas;
		}

		void StartAdding (DataViewWidget& view, const QString& prop)
		{
			auto model = view.GetModel ();
			const auto& datas = GetAddVariants (model, {});
			if (datas.isEmpty ())
				return;

			if (!QMetaObject::invokeMethod (model->parent (),
					"addRequested",
					Q_ARG (QString, prop),
					Q_ARG (QVariantList, datas)))
				qWarning () << "invokeMethod on "
						<< model->parent ()
						<< " for \"addRequested\" failed";
		}

		void StartModifying (DataViewWidget& view, const QString& prop)
		{
			const auto& selected = view.GetCurrentIndex ();
			if (!selected.isValid ())
				return;

			auto model = view.GetModel ();
			QVariantList existingData;
			for (int i = 0; i < model->columnCount (); ++i)
			{
				const auto& hData = model->headerData (i, Qt::Horizontal,
						DataSources::DataSourceRole::FieldType);
				const auto type = static_cast<DataSources::DataFieldType> (hData.value<int> ());
				if (type == DataSources::DataFieldType::None)
					continue;

				const auto& idx = model->index (selected.row (), i);
				const auto& editVar = idx.data (Qt::EditRole);
				existingData << (editVar.isNull () ? idx.data () : editVar);
			}

			const auto& datas = GetAddVariants (model, existingData);
			if (datas.isEmpty ())
				return;

			if (!QMetaObject::invokeMethod (model->parent (),
					"modifyRequested",
					Q_ARG (QString, prop),
					Q_ARG (int, selected.row ()),
					Q_ARG (QVariantList, datas)))
				qWarning () << "invokeMethod on "
						<< model->parent ()
						<< " for \"modifyRequested\" failed";
		}

		void StartRemoving (DataViewWidget& view, const QString& prop)
		{
			QModelIndexList indexes = view.GetSelectedRows ();
			if (!indexes.size ())
				return;

			std::sort (indexes.begin (), indexes.end (),
					[] (const QModelIndex& l, const QModelIndex& r) { return l.row () > r.row (); });
			auto model = view.GetModel ();

			if (!QMetaObject::invokeMethod (model->parent (),
					"removeRequested",
					Q_ARG (QString, prop),
					Q_ARG (QModelIndexList, indexes)))
				qWarning () << "invokeMethod on "
						<< model->parent ()
						<< " for \"removeRequested\" failed";
		}

		void SetupCommonButtons (DataViewWidget& view, const QString& prop)
		{
			QObject::connect (&view,
					&DataViewWidget::addRequested,
					[&view, prop] { StartAdding (view, prop); });
			QObject::connect (&view,
					&DataViewWidget::modifyRequested,
					[&view, prop] { StartModifying (view, prop); });
			QObject::connect (&view,
					&DataViewWidget::removeRequested,
					[&view, prop] { StartRemoving (view, prop); });
		}

		void AddCustomButtons (const QDomElement& item,
				DataViewWidget& view,
				const Util::XmlSettingsDialog& xsd,
				const QString& prop)
		{
			for (const auto& elem : Util::DomChildren (item, "button"_qs))
				view.AddCustomButton (elem.attribute ("id"_qs).toUtf8 (), xsd.GetLabel (elem));

			QObject::connect (&view,
					&DataViewWidget::customButtonReleased,
					[&view, prop] (const QByteArray& id)
					{
						const auto& selected = view.GetCurrentIndex ();
						const auto model = view.GetModel ();

						if (!QMetaObject::invokeMethod (model->parent (),
								"customButtonPressed",
								Q_ARG (QString, prop),
								Q_ARG (QByteArray, id),
								Q_ARG (int, selected.isValid () ? selected.row () : -1)))
							qWarning () << "invokeMethod on "
									<< model->parent ()
									<< " for \"handleCustomButton\" failed";
					});
		}

		DataViewWidget::Options GetOptions (const QDomElement& item)
		{
			DataViewWidget::Options options;
			if (item.attribute ("resizeToContents"_qs) == "true"_ql)
				options |= DataViewWidget::Option::Autoresize;
			if (item.attribute ("addEnabled"_qs) == "false"_ql)
				options |= DataViewWidget::Option::NoAdd;
			if (item.attribute ("modifyEnabled"_qs) == "false"_ql)
				options |= DataViewWidget::Option::NoModify;
			if (item.attribute ("removeEnabled"_qs) == "false"_ql)
				options |= DataViewWidget::Option::NoRemove;
			return options;
		}
	}

	ItemRepresentation HandleDataView (const ItemContext& ctx)
	{
		const auto& item = ctx.Elem_;
		const auto view = new DataViewWidget { GetOptions (item) };
		SetupCommonButtons (*view, ctx.Prop_);
		AddCustomButtons (item, *view, ctx.XSD_, ctx.Prop_);
		return
		{
			.Widget_ = view,
			.LabelPosition_ = LabelPosition::None,
			.DefaultValue_ = {},
			.Getter_ = {},
			.Setter_ = {},
			.DataSourceSetter_ = [view] (QAbstractItemModel& m) { view->SetModel (&m); },
		};
	}
}
