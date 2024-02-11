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

class QTreeView;

namespace LC
{
	class ItemHandlerFactory;

	class ItemHandlerTreeView : public ItemHandlerBase
	{
		ItemHandlerFactory *Factory_;
		QHash<QString, QTreeView*> Propname2TreeView_;
	public:
		ItemHandlerTreeView (ItemHandlerFactory*, Util::XmlSettingsDialog*);

		bool CanHandle (const QDomElement&) const;
		void Handle (const QDomElement&, QWidget*);
		QVariant GetValue (const QDomElement&, QVariant) const;
		void SetValue (QWidget*, const QVariant&) const;
	protected:
		QVariant GetObjectValue (QObject*) const;
	private:
		void SetDataSource (const QString&, QAbstractItemModel*);
	};
}
