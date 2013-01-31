/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "itemhandlerdataview.h"
#include <QGridLayout>
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QtDebug>
#include "../widgets/dataviewwidget.h"
#include "../filepicker.h"
#include "../itemhandlerfactory.h"
#include "../datasourceroles.h"

namespace LeechCraft
{
	ItemHandlerDataView::ItemHandlerDataView (ItemHandlerFactory *factory)
	: Factory_ (factory)
	{
	}

	bool ItemHandlerDataView::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "dataview";
	}

	void ItemHandlerDataView::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		DataViewWidget *view = new DataViewWidget (XSD_);
		connect (view,
				SIGNAL (addRequested ()),
				this,
				SLOT (handleAddRequested ()));
		connect (view,
				SIGNAL (removeRequested ()),
				this,
				SLOT (handleRemoveRequested ()));

		if (item.attribute ("addEnabled") == "false")
			view->DisableAddition ();
		if (item.attribute ("removeEnabled") == "false")
			view->DisableRemoval ();

		QString prop = item.attribute ("property");

		view->setObjectName (prop);

		Factory_->RegisterDatasourceSetter (prop,
				[this] (const QString& str, QAbstractItemModel *m, Util::XmlSettingsDialog*)
					{ SetDataSource (str, m); });
		Propname2DataView_ [prop] = view;

		lay->addWidget (view, lay->rowCount (), 0);
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

	void ItemHandlerDataView::SetDataSource (const QString& prop, QAbstractItemModel *model)
	{
		DataViewWidget *view = Propname2DataView_ [prop];
		if (!view)
		{
			qWarning () << Q_FUNC_INFO
					<< "dataview for property"
					<< prop
					<< "not found";
			return;
		}

		view->SetModel (model);
	}

	namespace
	{
		QWidget* GetEditor (DataSources::DataFieldType type, const QVariant& valuesInfo)
		{
			switch (type)
			{
			case DataSources::DataFieldType::Integer:
				return new QSpinBox ();
			case DataSources::DataFieldType::String:
			case DataSources::DataFieldType::Url:
				return new QLineEdit ();
			case DataSources::DataFieldType::LocalPath:
				return new FilePicker (FilePicker::Type::ExistingDirectory);
			case DataSources::DataFieldType::Enum:
			{
				auto box = new QComboBox;
				for (const auto& var : valuesInfo.toList ())
				{
					const auto& map = var.toMap ();
					box->addItem (map ["Icon"].value<QIcon> (),
							map ["Name"].toString (),
							map ["ID"]);
				}
				return box;
			}
			default:
				return 0;
			}
		}

		QVariant GetData (QWidget *editor, DataSources::DataFieldType type)
		{
			switch (type)
			{
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
			default:
				return QVariant ();
			}
		}
	}

	void ItemHandlerDataView::handleAddRequested ()
	{
		DataViewWidget *view = qobject_cast<DataViewWidget*> (sender ());
		if (!view)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a DataViewWidget"
					<< sender ();
			return;
		}

		QAbstractItemModel *model = view->GetModel ();

		struct ColumnInfo
		{
			DataSources::DataFieldType Type_;
			QVariant ValuesInfo_;
			QString Name_;
		};
		QList<ColumnInfo> infos;
		for (int i = 0, size = model->columnCount (); i < size; ++i)
		{
			const auto& hData = model->headerData (i, Qt::Horizontal,
					DataSources::DataSourceRole::FieldType);
			auto type = static_cast<DataSources::DataFieldType> (hData.value<int> ());
			if (type != DataSources::DataFieldType::None)
			{
				const auto& name = model->headerData (i, Qt::Horizontal, Qt::DisplayRole).toString ();
				const auto& values = model->headerData (i, Qt::Horizontal, DataSources::DataSourceRole::FieldValues);
				infos.push_back ({ type, values, name });
			}
		}

		QDialog dia (XSD_);
		QGridLayout *lay = new QGridLayout ();
		dia.setLayout (lay);
		for (const auto& info : infos)
		{
			QLabel *name = new QLabel (info.Name_);
			QWidget *w = GetEditor (info.Type_, info.ValuesInfo_);
			const int row = lay->rowCount ();
			lay->addWidget (name, row, 0, Qt::AlignRight);
			lay->addWidget (w, row, 1);
		}
		QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
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

		if (dia.exec () == QDialog::Accepted)
		{
			QVariantList datas;
			for (int i = 0, size = infos.size (); i < size; ++i)
			{
				QWidget *w = lay->itemAt (2 * i + 1)->widget ();
				datas << GetData (w, infos.at (i).Type_);
			}
			if (!QMetaObject::invokeMethod (model->parent (),
						"addRequested",
						Q_ARG (QString, view->objectName ()),
						Q_ARG (QVariantList, datas)))
				qWarning () << Q_FUNC_INFO
						<< "invokeMethod for \"addRequested\" failed";
		}
	}

	void ItemHandlerDataView::handleRemoveRequested ()
	{
		DataViewWidget *view = qobject_cast<DataViewWidget*> (sender ());
		if (!view)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a DataViewWidget"
					<< sender ();
			return;
		}
		if (!view->GetModel ())
		{
			qWarning () << Q_FUNC_INFO
					<< "model isn't ready";
			return;
		}

		QModelIndexList indexes = view->GetSelectedRows ();
		if (!indexes.size ())
			return;

		if (!QMetaObject::invokeMethod (view->GetModel ()->parent (),
					"removeRequested",
					Q_ARG (QString, view->objectName ()),
					Q_ARG (QModelIndexList, indexes)))
			qWarning () << Q_FUNC_INFO
					<< "invokeMethod for \"removeRequested\" failed";
	}
}
