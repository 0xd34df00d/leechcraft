/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlerbase.h"
#include <QHash>

class QListView;

namespace LC
{
	class ItemHandlerFactory;

	class ItemHandlerListView : public ItemHandlerBase
	{
		ItemHandlerFactory * const Factory_;
		QHash<QString, QListView*> Propname2Listview_;
	public:
		ItemHandlerListView (ItemHandlerFactory*, Util::XmlSettingsDialog*);

		bool CanHandle (const QDomElement&) const override;
		void Handle (const QDomElement&, QWidget*) override;
		QVariant GetValue (const QDomElement&, QVariant) const override;
		void SetValue (QWidget*, const QVariant&) const override;
	protected:
		QVariant GetObjectValue (QObject*) const override;
	private:
		void SetDataSource (const QString&, QAbstractItemModel*);
	};
}
