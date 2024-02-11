/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerlistview.h"
#include <QDomElement>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QtDebug>
#include "../itemhandlerfactory.h"

namespace LC
{
	ItemHandlerListView::ItemHandlerListView (ItemHandlerFactory *factory, Util::XmlSettingsDialog *xsd)
	: ItemHandlerBase { xsd }
	, Factory_ { factory }
	{
	}

	bool ItemHandlerListView::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "listview";
	}

	void ItemHandlerListView::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());

		QListView *list = new QListView (XSD_->GetWidget ());

		const QString& prop = item.attribute ("property");
		list->setObjectName (prop);

		Factory_->RegisterDatasourceSetter (prop,
				[this] (const QString& str, QAbstractItemModel *m, Util::XmlSettingsDialog*)
					{ SetDataSource (str, m); });
		Propname2Listview_ [prop] = list;

		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		list->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		list->setProperty ("SearchTerms", label->text ());

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignLeft);
		lay->addWidget (list, row + 1, 0);
	}

	QVariant ItemHandlerListView::GetValue (const QDomElement&, QVariant) const
	{
		return {};
	}

	void ItemHandlerListView::SetValue (QWidget*, const QVariant&) const
	{
	}

	QVariant ItemHandlerListView::GetObjectValue (QObject*) const
	{
		return {};
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
