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

	void ItemHandlerDataView::handleAddRequested ()
	{
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
