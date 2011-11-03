/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "itemhandlertreeview.h"
#include <boost/bind.hpp>
#include <QDomElement>
#include <QLabel>
#include <QTreeView>
#include <QGridLayout>
#include <QtDebug>
#include "../itemhandlerfactory.h"

namespace LeechCraft
{
	ItemHandlerTreeView::ItemHandlerTreeView (ItemHandlerFactory *factory)
	: Factory_ (factory)
	{
	}

	ItemHandlerTreeView::~ItemHandlerTreeView ()
	{

	}

	bool ItemHandlerTreeView::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "treeview";
	}

	void ItemHandlerTreeView::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());

		QTreeView *tree = new QTreeView (XSD_);
		tree->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		QString prop = item.attribute ("property");
		tree->setObjectName (prop);

		tree->setHeaderHidden (item.attribute ("hideHeader") == "true");

		Factory_->RegisterDatasourceSetter (prop,
				boost::bind (&ItemHandlerTreeView::SetDataSource, this, _1, _2));
		Propname2TreeView_ [prop] = tree;

		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		tree->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignLeft);
		lay->addWidget (tree, row + 1, 0);
	}

	QVariant ItemHandlerTreeView::GetValue (const QDomElement&, QVariant) const
	{
		return QVariant ();
	}

	void ItemHandlerTreeView::SetValue (QWidget*, const QVariant&) const
	{
	}

	void ItemHandlerTreeView::UpdateValue (QDomElement&, const QVariant&) const
	{
	}

	QVariant ItemHandlerTreeView::GetValue (QObject*) const
	{
		return QVariant ();
	}

	void ItemHandlerTreeView::SetDataSource (const QString& prop, QAbstractItemModel *model)
	{
		QTreeView *tree = Propname2TreeView_ [prop];
		if (!tree)
		{
			qWarning () << Q_FUNC_INFO
					<< "listview for property"
					<< prop
					<< "not found";
			return;
		}

		tree->setModel (model);
	}
}
