/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stringpathtrietest.h"
#include <QtTest>
#include <util/sll/prelude.h>
#include <stringpathtrie.h>

QTEST_APPLESS_MAIN (LC::Poshuku::WebEngineView::StringPathTrieTest)

namespace LC::Poshuku::WebEngineView
{
	using IntTrie = StringPathTrie<int>;

	using FindResult = IntTrie::FindResult;

	QVector<QStringRef> AsRefs (const QVector<QString>& lst)
	{
		return Util::Map (lst, [] (const auto& str) { return QStringRef { &str }; });
	}

	void StringPathTrieTest::testEmptyTrie ()
	{
		IntTrie trie;

		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz" })), FindResult { std::optional<int> {} });
	}

	void StringPathTrieTest::testEmptyQuery ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo", "bar", "baz" }), 10);

		QCOMPARE (trie.Find (AsRefs ({})), FindResult { std::optional<int> {} });
	}

	void StringPathTrieTest::testExactMatchSingle ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo", "bar", "baz" }), 10);

		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz" })), FindResult { std::optional<int> { 10 } });
	}

	void StringPathTrieTest::testExactMatchOverwriteSingle ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo", "bar", "baz" }), 10);
		trie.Add (AsRefs ({ "foo", "bar", "baz" }), 20);

		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz" })), FindResult { std::optional<int> { 20 } });
	}

	void StringPathTrieTest::testExactMatchMulti ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo", "bar", "baz1" }), 10);
		trie.Add (AsRefs ({ "foo", "bar", "baz2" }), 20);

		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz1" })), FindResult { std::optional<int> { 10 } });
		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz2" })), FindResult { std::optional<int> { 20 } });
	}

	void StringPathTrieTest::testExactMatchParentPre ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo", "bar" }), 10);
		trie.Add (AsRefs ({ "foo", "bar", "baz" }), 20);

		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar" })), FindResult { std::optional<int> { 10 } });
		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz" })), FindResult { std::optional<int> { 20 } });
	}

	void StringPathTrieTest::testExactMatchParentPost ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo", "bar", "baz" }), 20);
		trie.Add (AsRefs ({ "foo", "bar" }), 10);

		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar" })), FindResult { std::optional<int> { 10 } });
		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz" })), FindResult { std::optional<int> { 20 } });
	}

	void StringPathTrieTest::testPartialMatchLongerQuery ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo" }), 20);
		trie.Add (AsRefs ({ "foo", "bar" }), 10);

		QCOMPARE (trie.Find (AsRefs ({ "foo", "bar", "baz" })), (FindResult { std::optional<int> { 10 }, 1 }));
	}

	void StringPathTrieTest::testPartialMatchShorterQuery ()
	{
		IntTrie trie;
		trie.Add (AsRefs ({ "foo", "bar" }), 20);
		trie.Add (AsRefs ({ "foo", "bar", "baz" }), 10);

		QCOMPARE (trie.Find (AsRefs ({ "foo" })), FindResult { std::optional<int> {} });
	}
}
