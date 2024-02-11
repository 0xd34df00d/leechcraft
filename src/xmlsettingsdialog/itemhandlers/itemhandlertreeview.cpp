/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlertreeview.h"
#include <QDomElement>
#include <QLabel>
#include <QTreeView>
#include <QGridLayout>
#include <QtDebug>
#include "../itemhandlerfactory.h"

namespace LC
{
	ItemHandlerTreeView::ItemHandlerTreeView (ItemHandlerFactory *factory, Util::XmlSettingsDialog *xsd)
	: ItemHandlerBase { xsd }
	, Factory_ { factory }
	{
	}

	bool ItemHandlerTreeView::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "treeview";
	}

	void ItemHandlerTreeView::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());

		QTreeView *tree = new QTreeView (XSD_->GetWidget ());
		tree->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		QString prop = item.attribute ("property");
		tree->setObjectName (prop);

		tree->setHeaderHidden (item.attribute ("hideHeader") == "true");

		Factory_->RegisterDatasourceSetter (prop,
				[this] (const QString& str, QAbstractItemModel *m, Util::XmlSettingsDialog*)
					{ SetDataSource (str, m); });
		Propname2TreeView_ [prop] = tree;

		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		tree->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		tree->setProperty ("SearchTerms", label->text ());

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignLeft);
		lay->addWidget (tree, row + 1, 0, 1, -1);
	}

	QVariant ItemHandlerTreeView::GetValue (const QDomElement&, QVariant) const
	{
		return QVariant ();
	}

	void ItemHandlerTreeView::SetValue (QWidget*, const QVariant&) const
	{
	}

	QVariant ItemHandlerTreeView::GetObjectValue (QObject*) const
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
