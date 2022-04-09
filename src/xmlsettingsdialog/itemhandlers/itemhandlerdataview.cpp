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
#include <util/sll/prelude.h>
#include "../widgets/dataviewwidget.h"
#include "../filepicker.h"
#include "../itemhandlerfactory.h"
#include "../colorpicker.h"
#include "../datasourceroles.h"

namespace LC
{
	ItemHandlerDataView::ItemHandlerDataView (ItemHandlerFactory *factory, Util::XmlSettingsDialog *xsd)
	: ItemHandlerBase { xsd }
	, Factory_ { factory }
	{
	}

	bool ItemHandlerDataView::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "dataview";
	}

	void ItemHandlerDataView::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		DataViewWidget *view = new DataViewWidget (XSD_->GetWidget ());
		connect (view,
				SIGNAL (addRequested ()),
				this,
				SLOT (handleAddRequested ()));
		connect (view,
				SIGNAL (modifyRequested ()),
				this,
				SLOT (handleModifyRequested ()));
		connect (view,
				SIGNAL (removeRequested ()),
				this,
				SLOT (handleRemoveRequested ()));

		if (item.attribute ("addEnabled") == "false")
			view->DisableAddition ();
		if (item.attribute ("modifyEnabled") == "false")
			view->DisableModification ();
		if (item.attribute ("removeEnabled") == "false")
			view->DisableRemoval ();

		const bool resize = item.attribute ("resizeToContents") == "true";

		AddCustomButtons (item, view);

		QString prop = item.attribute ("property");

		view->setObjectName (prop);

		Factory_->RegisterDatasourceSetter (prop,
				[this, resize] (const QString& str, QAbstractItemModel *m, Util::XmlSettingsDialog*) -> void
					{ SetDataSource (str, m, resize); });
		Propname2DataView_ [prop] = view;

		lay->addWidget (view, lay->rowCount (), 0, 1, 2);
	}

	QVariant ItemHandlerDataView::GetValue (const QDomElement&, QVariant) const
	{
		return QVariant ();
	}

	void ItemHandlerDataView::SetValue (QWidget*, const QVariant&) const
	{
	}

	void ItemHandlerDataView::UpdateValue (QDomElement&, const QVariant&) const
	{
	}

	QVariant ItemHandlerDataView::GetObjectValue (QObject*) const
	{
		return QVariant ();
	}

	void ItemHandlerDataView::SetDataSource (const QString& prop, QAbstractItemModel *model, bool resize)
	{
		const auto view = Propname2DataView_ [prop];
		if (!view)
		{
			qWarning () << Q_FUNC_INFO
					<< "dataview for property"
					<< prop
					<< "not found";
			return;
		}

		view->SetModel (model);

		if (resize)
		{
			connect (model,
					SIGNAL (rowsInserted (QModelIndex, int, int)),
					view,
					SLOT (resizeColumns ()));
			connect (model,
					SIGNAL (dataChanged (QModelIndex, QModelIndex)),
					view,
					SLOT (resizeColumns ()));
		}
	}

	namespace
	{
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

			qWarning () << Q_FUNC_INFO
					<< "unhandled editor type"
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

			qWarning () << Q_FUNC_INFO
					<< "unknown field type"
					<< type;
			return {};
		}
	}

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
	}

	QVariantList ItemHandlerDataView::GetAddVariants (QAbstractItemModel *model, const QVariantList& existing)
	{
		const auto& infos = GetColumnInfos (model);

		QDialog dia (XSD_->GetWidget ());
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
		connect (buttons,
				SIGNAL (accepted ()),
				&dia,
				SLOT (accept ()));
		connect (buttons,
				SIGNAL (rejected ()),
				&dia,
				SLOT (reject ()));
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

	void ItemHandlerDataView::AddCustomButtons (const QDomElement& item, DataViewWidget *view)
	{
		auto elem = item.firstChildElement ("button");
		while (!elem.isNull ())
		{
			const auto& text = XSD_->GetLabel (elem);
			const auto& id = elem.attribute ("id").toUtf8 ();

			view->AddCustomButton (id, text);

			elem = elem.nextSiblingElement ("button");
		}

		connect (view,
				SIGNAL (customButtonReleased (QByteArray)),
				this,
				SLOT (handleCustomButton (QByteArray)));
	}

	void ItemHandlerDataView::handleAddRequested ()
	{
		const auto view = qobject_cast<DataViewWidget*> (sender ());

		auto model = view->GetModel ();
		const auto& datas = GetAddVariants (model);
		if (datas.isEmpty ())
			return;

		if (!QMetaObject::invokeMethod (model->parent (),
					"addRequested",
					Q_ARG (QString, view->objectName ()),
					Q_ARG (QVariantList, datas)))
			qWarning () << Q_FUNC_INFO
					<< "invokeMethod on "
					<< model->parent ()
					<< " for \"addRequested\" failed";
	}

	void ItemHandlerDataView::handleModifyRequested ()
	{
		const auto view = qobject_cast<DataViewWidget*> (sender ());

		const auto& selected = view->GetCurrentIndex ();
		if (!selected.isValid ())
			return;

		auto model = view->GetModel ();
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
					Q_ARG (QString, view->objectName ()),
					Q_ARG (int, selected.row ()),
					Q_ARG (QVariantList, datas)))
			qWarning () << Q_FUNC_INFO
					<< "invokeMethod on "
					<< model->parent ()
					<< " for \"modifyRequested\" failed";
	}

	void ItemHandlerDataView::handleRemoveRequested ()
	{
		DataViewWidget *view = qobject_cast<DataViewWidget*> (sender ());
		if (!view->GetModel ())
		{
			qWarning () << Q_FUNC_INFO
					<< "model isn't ready";
			return;
		}

		QModelIndexList indexes = view->GetSelectedRows ();
		if (!indexes.size ())
			return;

		std::sort (indexes.begin (), indexes.end (),
				[] (const QModelIndex& l, const QModelIndex& r)
					{ return l.row () > r.row (); });
		auto model = view->GetModel ();

		if (!QMetaObject::invokeMethod (model->parent (),
					"removeRequested",
					Q_ARG (QString, view->objectName ()),
					Q_ARG (QModelIndexList, indexes)))
			qWarning () << Q_FUNC_INFO
					<< "invokeMethod on "
					<< model->parent ()
					<< " for \"removeRequested\" failed";
	}

	void ItemHandlerDataView::handleCustomButton (const QByteArray& id)
	{
		const auto view = qobject_cast<DataViewWidget*> (sender ());
		if (!view->GetModel ())
		{
			qWarning () << Q_FUNC_INFO
					<< "model isn't ready";
			return;
		}

		const auto& selected = view->GetCurrentIndex ();
		const auto model = view->GetModel ();

		if (!QMetaObject::invokeMethod (model->parent (),
					"customButtonPressed",
					Q_ARG (QString, view->objectName ()),
					Q_ARG (QByteArray, id),
					Q_ARG (int, selected.isValid () ? selected.row () : -1)))
			qWarning () << Q_FUNC_INFO
					<< "invokeMethod on "
					<< model->parent ()
					<< " for \"handleCustomButton\" failed";
	}
}
