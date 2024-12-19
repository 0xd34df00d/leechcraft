/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVariant>
#include <QDomElement>

class QAbstractItemModel;

namespace LC::Util
{
	class XmlSettingsDialog;
}

namespace LC
{
	enum class LabelPosition
	{
		Default,
		Wrap,
		None,
	};

	using ItemValueSetter = std::function<void (QVariant)>;
	using ItemValueGetter = std::function<QVariant ()>;

	using DataSourceSetter = std::function<void (QAbstractItemModel&)>;

	struct ItemRepresentation
	{
		QWidget *Widget_;
		std::optional<QString> Label_ {};
		std::optional<QStringList> SearchTerms_ {};

		LabelPosition LabelPosition_ = LabelPosition::Default;

		QVariant DefaultValue_ {};
		ItemValueGetter Getter_ {};
		ItemValueSetter Setter_ {};

		DataSourceSetter DataSourceSetter_ {};
	};

	struct ItemContext
	{
		QDomElement Elem_;
		QString Label_;
		Util::XmlSettingsDialog& XSD_;

		QString Prop_;
		QString Default_;

		std::function<void ()> MarkChanged_;
	};

	auto SetChangedSignal (const ItemContext& ctx, auto *widget, const auto& signal)
	{
		QObject::connect (widget,
				signal,
				[markChanged = ctx.MarkChanged_] { markChanged (); });
	}

	using ItemHandler = ItemRepresentation (*) (const ItemContext&);
}
