/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "itemhandlerlistview.h"
#include <boost/bind.hpp>
#include <QDomElement>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QtDebug>
#include "../itemhandlerfactory.h"

namespace LeechCraft
{
	ItemHandlerListView::ItemHandlerListView (ItemHandlerFactory *factory)
	: Factory_ (factory)
	{
	}
    
	ItemHandlerListView::~ItemHandlerListView ()
	{
	}
	
	bool ItemHandlerListView::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "listview";
	}
	
	void ItemHandlerListView::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		
		QListView *list = new QListView (XSD_);
		
		QString prop = item.attribute ("property");
		list->setObjectName (prop);
		
		Factory_->RegisterDatasourceSetter (prop,
				boost::bind (&ItemHandlerListView::SetDataSource, this, _1, _2));
		Propname2Listview_ [prop] = list;
		
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		list->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));
		
		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignLeft);
		lay->addWidget (list, row + 1, 0);
	}
	
	QVariant ItemHandlerListView::GetValue (const QDomElement&, QVariant) const
	{
		return QVariant ();
	}

	void ItemHandlerListView::SetValue (QWidget*, const QVariant&) const
	{
	}

	void ItemHandlerListView::UpdateValue (QDomElement&, const QVariant&) const
	{
	}

	QVariant ItemHandlerListView::GetValue (QObject*) const
	{
		return QVariant ();
	}
	
	void ItemHandlerListView::SetDataSource (const QString& prop, QAbstractItemModel *model)
	{
		QListView *list = Propname2Listview_ [prop];
		if (!list)
		{
			qWarning () << Q_FUNC_INFO
					<< "listview for property"
					<< prop
					<< "not found";
			return;
		}

		list->setModel (model);
	}
}
