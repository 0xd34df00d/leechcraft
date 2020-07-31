/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandleroptionssetvalue.h"
#include <QHash>

class QComboBox;

namespace LC
{
	class ItemHandlerFactory;

	class ItemHandlerCombobox : public ItemHandlerOptionsSetValue
	{
		ItemHandlerFactory * const Factory_;

		QHash<QString, QComboBox*> Propname2Combobox_;
		QHash<QString, QDomElement> Propname2Item_;
	public:
		ItemHandlerCombobox (ItemHandlerFactory*, Util::XmlSettingsDialog*);

		bool CanHandle (const QDomElement&) const override;
		void Handle (const QDomElement&, QWidget*) override;
		void SetValue (QWidget*, const QVariant&) const override;
	protected:
		QVariant GetObjectValue (QObject*) const override;
	private:
		void SetDataSource (const QString&, QAbstractItemModel*, Util::XmlSettingsDialog*);
	};
}
