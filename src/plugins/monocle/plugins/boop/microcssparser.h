/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QHash>
#include <QString>
#include <QVector>

namespace LC::Monocle::Boop::MicroCSS
{
	struct Rule
	{
		QString Property_;
		QString Value_;
	};

	QDebug operator<< (QDebug, const Rule&);

	struct AtSelector
	{
		QString Sel_;

		bool operator== (const AtSelector&) const = default;
	};

	struct TagSelector
	{
		QString Sel_;

		bool operator== (const TagSelector&) const = default;
	};

	struct ClassSelector
	{
		QString Sel_;

		bool operator== (const ClassSelector&) const = default;
	};

	struct ComplexSelector
	{
		QString Sel_;

		bool operator== (const ComplexSelector&) const = default;
	};

	using Selector = std::variant<AtSelector, TagSelector, ClassSelector, ComplexSelector>;

	size_t qHash (const Selector&);

	struct Stylesheet
	{
		QHash<Selector, QVector<Rule>> Selectors_;

		Stylesheet& operator+= (const Stylesheet&);
	};

	Stylesheet Parse (QStringView str, const std::function<bool (const Selector&)>& selectorFilter);
}
