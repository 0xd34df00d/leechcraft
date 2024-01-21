/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "microcssparser.h"
#include <algorithm>
#include <deque>
#include <QDebug>
#include <QRegularExpression>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <util/monocle/types.h>

namespace LC::Monocle::Boop::MicroCSS
{
	namespace
	{
		QString ToString (auto begin, auto end)
		{
			QString result;
			const auto totalLength = std::accumulate (begin, end, 0,
					[] (int acc, QStringView str) { return acc + str.size (); });
			result.reserve (totalLength + (end - begin));

			for (auto it = begin; it != end; ++it)
			{
				if (!result.isEmpty ())
					result += ' ';
				result += *it;
			}

			return result;
		}

		template<char... Items>
		auto FirstIndexOf (QStringView str)
		{
			return static_cast<qsizetype> (std::min ({ static_cast<size_t> (str.indexOf (Items))... }));
		}

		QVector<QStringView> Tokenize (const QList<QStringView>& wordsList)
		{
			std::deque<QStringView> words { wordsList.begin (), wordsList.end () };

			QVector<QStringView> result;
			result.reserve (words.size () * 2);
			while (!words.empty ())
			{
				auto word = words.front ();
				words.pop_front ();

				const auto sepPos = FirstIndexOf<'{', '}', ':', ';'> (word);
				if (sepPos == -1)
				{
					result << word;
					continue;
				}

				auto leftSub = word.left (sepPos);
				if (!leftSub.isEmpty ())
					result << leftSub;
				result << word.mid (sepPos, 1);
				auto rightSub = word.mid (sepPos + 1);
				if (!rightSub.isEmpty ())
					words.push_front (rightSub);
			}
			return result;
		}

		template<typename It>
		std::pair<It, std::optional<Rule>> TryParseRule (It begin, It end)
		{
			const auto nameSep = std::find (begin, end, ':');
			if (nameSep == end || nameSep + 1 == end)
				return { end, {} };

			const auto semiSep = std::find (nameSep + 1, end, ';');
			Rule rule;
			rule.Property_ = ToString (begin, nameSep);
			rule.Value_ = ToString (nameSep + 1, semiSep);
			return { semiSep == end ? semiSep : semiSep + 1, std::move (rule) };
		}

		template<typename It>
		std::pair<It, std::optional<QString>> TryParseSelector (It it, It end)
		{
			const auto blockStartPos = std::find (it, end, '{');
			if (blockStartPos == end)
				return { end, {} };

			return { blockStartPos + 1, ToString (it, blockStartPos) };
		}

		QVector<Rule> TryParseSelectorBlock (auto it, auto end)
		{
			QVector<Rule> result;
			result.reserve (std::count (it, end, ';') + 1);
			while (it != end)
			{
				auto [next, maybeRule] = TryParseRule (it, end);
				if (maybeRule)
					result << *maybeRule;
				it = next;
			}
			return result;
		}

		template<typename It>
		std::optional<It> TryIgnoreAtRule (It it, It end)
		{
			if (it == end || !it->startsWith ('@'))
				return {};

			const auto nextSemi = std::find (it + 1, end, ';');
			const auto nextBlock = std::find (it + 1, end, '{');
			if (nextSemi == end || nextBlock == end)
				return {};

			if (nextSemi < nextBlock)
				return { nextSemi + 1 };

			return {};
		}

		template<typename It>
		std::optional<It> TryIgnoreAtRules (It it, It end)
		{
			std::optional<It> prevIgnored;
			std::optional<It> currIgnored;
			while ((currIgnored = TryIgnoreAtRule (it, end)))
			{
				it = *currIgnored;
				prevIgnored = currIgnored;
			}
			return prevIgnored;
		}

		SingleSelector ParseSingleSelector (QStringRef part)
		{
			if (part.startsWith ('@'))
				return AtSelector { part.mid (1).toString () };

			auto tagAndClasses = part.split ('.', Qt::KeepEmptyParts);

			Q_ASSERT (!tagAndClasses.isEmpty ());

			const auto tag = tagAndClasses [0];

			// no dots, it's a class
			if (tagAndClasses.size () == 1)
				return TagSelector { tag.toString () };

			// there are only two components: a (potentially empty) tag and a class
			if (tagAndClasses.size () == 2)
			{
				const auto& klass = tagAndClasses [1];
				if (tag.isEmpty ())
					return ClassSelector { klass.toString () };
				else
					return TagClassSelector { .Tag_ = tag.toString (), .Class_ = klass.toString () };
			}

			// there are more components: a (potentially empty) tag name and several classes
			tagAndClasses.pop_front ();
			const auto& classes = Util::Map (tagAndClasses, &QStringRef::toString);
			return [tag = tag.toString (), classes] (const StylingContextElement& elem)
			{
				if (!tag.isEmpty () && tag != elem.Tag_)
					return false;

				return std::all_of (classes.begin (), classes.end (),
						[&] (auto klass) { return elem.Classes_.contains (klass); });
			};
		}

		auto ParseSelectors (const QString& rawStr)
		{
			QVector<Selector> result;
			result.reserve (rawStr.count (',') + 1);

			for (auto sub : rawStr.splitRef (',', Qt::SkipEmptyParts))
			{
				sub = sub.trimmed ();
				QVector<SingleSelector> parts;
				parts.reserve (sub.count (' ') + 1);
				for (const auto& part : sub.split (' ', Qt::SkipEmptyParts))
					parts << ParseSingleSelector (part);

				if (parts.isEmpty ())
					continue;

				auto head = parts.back ();
				parts.pop_back ();
				std::reverse (parts.begin (), parts.end ());
				result << Selector { head, std::move (parts) };
			}

			return result;
		}
	}

	Stylesheet Parse (QStringView text)
	{
		Stylesheet result;

		auto chunks = Tokenize (text.split (QRegularExpression { "\\s+"_qs }, Qt::SkipEmptyParts));
		for (auto pos = chunks.begin (); pos != chunks.end (); )
		{
			if (const auto afterAtRules = TryIgnoreAtRules (pos, chunks.end ()))
				pos = *afterAtRules;

			auto [blockStart, maybeSelector] = TryParseSelector (pos, chunks.end ());
			if (!maybeSelector)
				return result;

			auto blockEnd = std::find (blockStart, chunks.end (), '}');
			if (blockEnd == chunks.end ())
				return result;

			const auto& selectors = ParseSelectors (*maybeSelector);
			const auto& block = TryParseSelectorBlock (blockStart, blockEnd);
			for (const auto& selector : selectors)
				result.Selectors_.push_back ({ selector, block });

			pos = blockEnd + 1;
		}

		return result;
	}

	Stylesheet& Stylesheet::operator+= (const Stylesheet& other)
	{
		Selectors_ += other.Selectors_;
		return *this;
	}

	QDebug operator<< (QDebug debug, const Rule& rule)
	{
		QDebugStateSaver saver { debug };
		debug.nospace ().noquote () << rule.Property_ << ": " << rule.Value_;
		return debug;
	}
}
