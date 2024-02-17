/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QSet>
#include <QVector>
#include "itemhandlers/itemhandlerbase.h"

class QDomElement;
class QFormLayout;

namespace LC
{
	class ItemHandlerFactory : public QObject
	{
		Util::XmlSettingsDialog& XSD_;
		const QHash<QByteArray, ItemHandler> Handlers_;

		struct PropInfo
		{
			QWidget *Widget_ = nullptr;
			QString Prop_;
			ItemValueGetter Getter_;
			ItemValueSetter Setter_;
			QVariant DefaultValue_;

			explicit operator bool () const;
		};

		QHash<QString, PropInfo> Prop2Info_;
		QHash<QString, QVariant> Prop2NewVal_;
		QHash<QString, DataSourceSetter> Prop2DataSourceSetter_;

		QVector<QWidget*> CustomWidgets_;

		mutable QSet<QString> ExpectedPropChanges_;
	public:
		ItemHandlerFactory (Util::XmlSettingsDialog*);

		std::optional<QVariant> Handle (const QDomElement& element, QFormLayout& layout);

		void SetValue (const QString& propName, const QVariant& value) const;

		void Accept ();
		void Reject ();

		void SetDataSource (const QString&, QAbstractItemModel*);
		void SetCustomWidget (const QString&, QWidget*);
	private:
		void SetReprValue (const QString&) const;
		void MarkChanged (const QString& propName);
		[[nodiscard]] auto ExpectPropChange (const QString&) const;
	};
}
