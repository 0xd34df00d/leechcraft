/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlerstringgetvalue.h"

namespace LC
{
	template<typename WidgetType, typename ValueType>
	class ItemHandlerSpinboxBase : public ItemHandlerStringGetValue
	{
	public:
		using Converter_t = ValueType (*) (const QString&);
	private:
		Converter_t Converter_;
		QString ElementType_;
	public:
		ItemHandlerSpinboxBase (Converter_t cvt, const QString& etype, Util::XmlSettingsDialog *xsd);

		bool CanHandle (const QDomElement& element) const override;
		void Handle (const QDomElement& item, QWidget *pwidget) override;
		void SetValue (QWidget *widget, const QVariant& value) const override;
	protected:
		QVariant GetObjectValue (QObject *object) const override;
	};
}
