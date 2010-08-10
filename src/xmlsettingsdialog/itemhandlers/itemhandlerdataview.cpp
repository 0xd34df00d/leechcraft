/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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
#include <boost/bind.hpp>
#include <QFormLayout>
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QtDebug>
#include "../widgets/dataviewwidget.h"
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
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		DataViewWidget *view = new DataViewWidget (XSD_);
		connect (view,
				SIGNAL (addRequested ()),
				this,
				SLOT (handleAddRequested ()));
		connect (view,
				SIGNAL (removeRequested ()),
				this,
				SLOT (handleRemoveRequested ()));

		QString prop = item.attribute ("property");

		view->setObjectName (prop);

		Factory_->RegisterDatasourceSetter (prop,
				boost::bind (&ItemHandlerDataView::SetDataSource, this, _1, _2));
		Propname2DataView_ [prop] = view;

		lay->addRow (view);
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

	QVariant ItemHandlerDataView::GetValue (QObject*) const
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
		QWidget* GetEditor (DataSources::DataFieldType type)
		{
			switch (type)
			{
			case DataSources::DFTInteger:
				return new QSpinBox ();
			case DataSources::DFTString:
			case DataSources::DFTUrl:
				return new QLineEdit ();
			default:
				return 0;
			}
		}

		QVariant GetData (QWidget *editor, DataSources::DataFieldType type)
		{
			switch (type)
			{
			case DataSources::DFTInteger:
				return qobject_cast<QSpinBox*> (editor)->value ();
			case DataSources::DFTString:
			case DataSources::DFTUrl:
				return qobject_cast<QLineEdit*> (editor)->text ();
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
		QList<DataSources::DataFieldType> types;
		QStringList names;
		for (int i = 0, size = model->columnCount (); i < size; ++i)
		{
			DataSources::DataFieldType type = static_cast<DataSources::DataFieldType> (model->
						headerData (i, Qt::Horizontal, DataSources::DSRFieldType).value<int> ());
			if (type != DataSources::DFTNone)
			{
				types << type;
				names << model->headerData (i, Qt::Horizontal, Qt::DisplayRole).toString ();
			}
		}

		QDialog *dia = new QDialog (XSD_);
		QFormLayout *lay = new QFormLayout ();
		dia->setLayout (lay);
		for (int i = 0, size = types.size (); i < size; ++i)
		{
			QString name = names.at (i);
			DataSources::DataFieldType type = types.at (i);
			QWidget *w = GetEditor (type);
			lay->addRow (name, w);
		}
		QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
				Qt::Horizontal, dia);
		connect (buttons,
				SIGNAL (accepted ()),
				dia,
				SLOT (accept ()));
		connect (buttons,
				SIGNAL (rejected ()),
				dia,
				SLOT (reject ()));
		lay->addRow (buttons);

		if (dia->exec () == QDialog::Accepted)
		{
			QVariantList datas;
			for (int i = 0, size = types.size (); i < size; ++i)
			{
				QWidget *w = lay->itemAt (i, QFormLayout::FieldRole)->widget ();
				QVariant data = GetData (w, types.at (i));
				datas << data;
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
