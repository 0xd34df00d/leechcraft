/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include <QHash>
#include <QString>
#include <QVector>

namespace LC::Monocle
{
	struct StylingContextElement;
}

namespace LC::Monocle::Boop::MicroCSS
{
	struct Rule
	{
		QString Property_;
		QString Value_;

		bool operator== (const Rule&) const = default;
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

	quint64 qHash (const TagSelector&);

	struct ClassSelector
	{
		QString Class_;

		bool operator== (const ClassSelector&) const = default;
	};

	quint64 qHash (const ClassSelector&);

	struct TagClassSelector
	{
		QString Tag_;
		QString Class_;

		bool operator== (const TagClassSelector&) const = default;
	};

	quint64 qHash (const TagClassSelector&);

	struct ManyClassesSelector
	{
		QString Tag_;
		QVector<QString> Classes_;

		bool operator== (const ManyClassesSelector&) const = default;
	};

	quint64 qHash (const ManyClassesSelector&);

	using SingleSelector = std::variant<AtSelector, TagSelector, ClassSelector, TagClassSelector, ManyClassesSelector>;

	struct Selector
	{
		SingleSelector Head_;
		// The context is stored in the reversed order: the rightmost (sub)selector is the first one.
		QVector<SingleSelector> Context_;
	};

	using ComplexRules_t = QVector<std::pair<Selector, QVector<Rule>>>;

	struct Stylesheet
	{
		std::optional<SingleSelector> Scope_;

		QHash<TagSelector, QVector<Rule>> ByTag_;
		QHash<ClassSelector, QVector<Rule>> ByClass_;
		QHash<TagClassSelector, QVector<Rule>> ByTagAndClass_;

		QHash<QString, QVector<std::pair<ManyClassesSelector, QVector<Rule>>>> ManyClassesByTag_;

		QHash<QString, ComplexRules_t> ComplexByTag_;
		ComplexRules_t Others_;
	};

	Stylesheet Parse (QStringView str);
}
