/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pslhandler.h"
#include <algorithm>
#include <util/sll/tokenize.h>

namespace LC::Poshuku::CleanWeb
{
	PslHandler::PslHandler (QStringView contents)
	: Tlds_ { ParseFile (contents) }
	{
	}

	int PslHandler::GetTldCountHost (QStringView host) const
	{
		auto components = host.split ('.', Qt::SkipEmptyParts);
		if (components.isEmpty ())
			return 1;

		int curLength = 0;
		int bestLength = 1;

		auto curTrie = &Tlds_;
		for (auto rit = components.rbegin (); rit != components.rend (); ++rit, ++curLength)
		{
			auto childTrie = curTrie->GetChild (*rit);
			// if the current component isn't found, fall back to checking '*'
			if (!childTrie)
				childTrie = curTrie->GetChild (u"*");

			// if neither is found, just return the best one we've seen
			if (!childTrie)
				return bestLength;

			// if we've found a component, but it's an exception, then the TLD is one up,
			// and there's no sense in looking further
			if (childTrie->GetValue () == Kind::Exception)
				return curLength;

			if (childTrie->GetValue ())
				bestLength = curLength + 1;

			curTrie = childTrie;
		}

		// we've come this far, so the whole domain is the TLD
		return components.size ();
	}

	QString PslHandler::GetRegistrableDomain (QStringView host) const
	{
		const auto& lowerHost = host.toString ().toLower ();
		const auto& labels = QStringView { lowerHost }.split ('.', Qt::SkipEmptyParts);
		const auto count = std::min<qsizetype> (labels.size (), GetTldCountHost (lowerHost) + 1);

		QString result;
		for (const auto label : labels.last (count))
		{
			if (!result.isEmpty ())
				result += '.';
			result += label;
		}
		return result;
	}

	Util::StringPathTrie<PslHandler::Kind> PslHandler::ParseFile (QStringView contents)
	{
		Util::StringPathTrie<Kind> trie;

		for (auto line : Util::Tokenize { contents, '\n' })
		{
			line = line.left (line.indexOf (' '));
			if (line.isEmpty () || line.startsWith (u"//"))
				continue;

			auto kind = Kind::Rule;
			if (line [0] == '!')
			{
				kind = Kind::Exception;
				line = line.mid (1);
			}

			Util::Tokenize split { line, '.' };
			trie.Add (split.rbegin (), split.rend (), kind);
		}

		return trie;
	}
}
