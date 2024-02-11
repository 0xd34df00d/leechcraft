/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlerbase.h"
#include <QHash>

namespace LC
{
	class ItemHandlerFactory;
	class DataViewWidget;

	class ItemHandlerDataView : public ItemHandlerBase
	{
		Q_OBJECT

		ItemHandlerFactory *Factory_;
		QHash<QString, DataViewWidget*> Propname2DataView_;
	public:
		ItemHandlerDataView (ItemHandlerFactory*, Util::XmlSettingsDialog*);

		bool CanHandle (const QDomElement&) const override;
		void Handle (const QDomElement&, QWidget*) override;
		QVariant GetValue (const QDomElement&, QVariant) const override;
		void SetValue (QWidget*, const QVariant&) const override;
	protected:
		QVariant GetObjectValue (QObject*) const override;
	private:
		void SetDataSource (const QString&, QAbstractItemModel*, bool);

		QVariantList GetAddVariants (QAbstractItemModel*, const QVariantList& = QVariantList ());

		void AddCustomButtons (const QDomElement&, DataViewWidget*);
	private slots:
		void handleAddRequested ();
		void handleModifyRequested ();
		void handleRemoveRequested ();

		void handleCustomButton (const QByteArray&);
	};
}
