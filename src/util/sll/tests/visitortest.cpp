/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "visitortest.h"
#include <QtTest>
#include <visitor.h>

QTEST_MAIN (LC::Util::VisitorTest)

namespace LC
{
namespace Util
{
	using Variant_t = std::variant<int, char, std::string, QString, double, float>;

	struct S1
	{
		int field1;
		double field2;
	};

	struct S2
	{
		int field1;
		double field2;
	};

	using SVariant_t = std::variant<S1, S2>;

	void VisitorTest::testBasicVisitor ()
	{
		Variant_t v { 'a' };
		const auto& res = Visit (v,
					[] (char) { return true; },
					[] (int) { return false; },
					[] (std::string) { return false; },
					[] (QString) { return false; },
					[] (double) { return false; },
					[] (float) { return false; });
		QCOMPARE (res, true);
	}

	void VisitorTest::testBasicVisitorGenericFallback ()
	{
		Variant_t v { 'a' };
		const auto& res = Visit (v,
					[] (char) { return true; },
					[] (int) { return false; },
					[] (auto) { return false; });
		QCOMPARE (res, true);
	}

	void VisitorTest::testBasicVisitorCoercion ()
	{
		Variant_t v { 'a' };
		const auto& res = Visit (v,
					[] (int) { return true; },
					[] (std::string) { return false; },
					[] (QString) { return false; },
					[] (double) { return false; },
					[] (float) { return false; });
		QCOMPARE (res, true);
	}

	void VisitorTest::testBasicVisitorCoercionGenericFallback ()
	{
		Variant_t v { 'a' };
		const auto& res = Visit (v,
					[] (int) { return false; },
					[] (QString) { return false; },
					[] (auto) { return true; });
		QCOMPARE (res, true);
	}

#define NC nc = std::unique_ptr<int> {}

	void VisitorTest::testNonCopyableFunctors ()
	{
		Variant_t v { 'a' };
		const auto& res = Visit (v,
					[NC] (char) { return true; },
					[NC] (int) { return false; },
					[NC] (std::string) { return false; },
					[NC] (QString) { return false; },
					[NC] (double) { return false; },
					[NC] (float) { return false; });
		QCOMPARE (res, true);
	}
#undef NC

	void VisitorTest::testAcceptsRValueRef ()
	{
		const auto& res = Visit (Variant_t { 'a' },
				[] (char) { return true; },
				[] (auto) { return false; });
		QCOMPARE (res, true);
	}

	void VisitorTest::testLValueRef ()
	{
		Variant_t v { 'a' };
		int ref = 0;
		auto& res = Visit (v, [&ref] (auto) -> int& { return ref; });
		res = 10;
		QCOMPARE (ref, 10);
	}

	void VisitorTest::testLValueRef2 ()
	{
		SVariant_t v { S1 { 0, 0 } };
		Visit (v, [] (auto& s) -> int& { return s.field1; }) = 10;
		const auto& res = Visit (v, [] (const auto& s) -> const int& { return s.field1; });
		QCOMPARE (res, 10);
	}

	void VisitorTest::testPrepareVisitor ()
	{
		Variant_t v { 'a' };
		Visitor visitor
		{
			[] (char) { return true; },
			[] (int) { return false; },
			[] (std::string) { return false; },
			[] (QString) { return false; },
			[] (double) { return false; },
			[] (float) { return false; }
		};

		const auto& res = visitor (v);
		QCOMPARE (res, true);
	}

	void VisitorTest::testPrepareVisitorConst ()
	{
		const Variant_t v { 'a' };
		Visitor visitor
		{
			[] (char) { return true; },
			[] (int) { return false; },
			[] (std::string) { return false; },
			[] (QString) { return false; },
			[] (double) { return false; },
			[] (float) { return false; }
		};

		const auto& res = visitor (v);
		QCOMPARE (res, true);
	}

	void VisitorTest::testPrepareVisitorRValue ()
	{
		Visitor visitor
		{
			[] (char) { return true; },
			[] (int) { return false; },
			[] (std::string) { return false; },
			[] (QString) { return false; },
			[] (double) { return false; },
			[] (float) { return false; }
		};

		const auto& res = visitor (Variant_t { 'a' });
		QCOMPARE (res, true);
	}

	void VisitorTest::testPrepareVisitorFinally ()
	{
		Variant_t v { 'a' };

		bool fin = false;

		auto visitor = Visitor
		{
			[] (char) { return true; },
			[] (auto) { return false; }
		}.Finally ([&fin] { fin = true; });

		const auto& res = visitor (v);
		QCOMPARE (res, true);
		QCOMPARE (fin, true);
	}

	void VisitorTest::testPrepareJustAutoVisitor ()
	{
		using Variant_t = std::variant<int, double, float>;

		Visitor visitor
		{
			[] (auto e) { return std::to_string (e); }
		};

		const auto& res = visitor (Variant_t { 10 });
		QCOMPARE (res, std::string { "10" });
	}

	void VisitorTest::testPrepareRecursiveVisitor ()
	{
		using SubVariant_t = std::variant<int, double, float>;
		using Variant_t = std::variant<SubVariant_t, QString>;

		Visitor visitor
		{
			[] (const QString& str) { return str; },
			Visitor { [] (auto e) { return QString::fromStdString (std::to_string (e)); } }
		};

		const auto& res = visitor (Variant_t { SubVariant_t { 10 } });
		QCOMPARE (res, QString { "10" });
	}

	void VisitorTest::testPrepareVisitorMutable ()
	{
		Variant_t v { 'a' };
		Visitor visitor
		{
			[] (int) mutable { return true; },
			[] (auto) mutable { return false; }
		};

		const auto& res = visitor (v);
		QCOMPARE (res, false);
	}
}
}
