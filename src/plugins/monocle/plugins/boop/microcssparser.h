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

namespace LC::Monocle
{
	struct StylingContext;
}

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
		QString Tag_;

		bool operator== (const TagSelector&) const = default;
	};

	struct ClassSelector
	{
		QString Class_;

		bool operator== (const ClassSelector&) const = default;
	};

	struct TagClassSelector
	{
		QString Tag_;
		QString Class_;

		bool operator== (const TagClassSelector&) const = default;
	};

	using ComplexSelector = std::function<bool (const StylingContext&)>;

	using SingleSelector = std::variant<AtSelector, TagSelector, ClassSelector, TagClassSelector, ComplexSelector>;

	struct Selector
	{
		SingleSelector Head_;
		QVector<SingleSelector> Context_;
	};

	struct Stylesheet
	{
		QVector<std::pair<Selector, QVector<Rule>>> Selectors_;

		Stylesheet& operator+= (const Stylesheet&);
	};

	Stylesheet Parse (QStringView str);
}
