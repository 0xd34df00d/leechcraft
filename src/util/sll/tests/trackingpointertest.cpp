/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trackingpointertest.h"
#include <memory>
#include <type_traits>
#include <QtTest>
#include "trackingpointer.h"

QTEST_MAIN (LC::Util::TrackingPointerTest)

namespace LC::Util
{
	namespace
	{
		class ITrackable
		{
		public:
			virtual ~ITrackable () = default;

			virtual QObject* GetQObject () = 0;
			virtual int GetValue () const = 0;
		};

		class TrackableObject
			: public QObject
			, public ITrackable
		{
			int Value_;
		public:
			explicit TrackableObject (int value = 0)
			: Value_ { value }
			{
			}

			QObject* GetQObject () override
			{
				return this;
			}

			int GetValue () const override
			{
				return Value_;
			}
		};

		class ExternallyBacked
		{
			QObject *Anchor_;
		public:
			explicit ExternallyBacked (QObject *anchor)
			: Anchor_ { anchor }
			{
			}

			QObject* Anchor () const
			{
				return Anchor_;
			}
		};

		QObject* AnchorOf (ExternallyBacked *backed)
		{
			return backed->Anchor ();
		}
	}

	static_assert (std::is_convertible_v<ITrackable*, TrackingPointer<ITrackable>>,
			"a raw interface pointer should implicitly convert into a TrackingPointer");
	static_assert (!std::is_convertible_v<TrackingPointer<ITrackable>, ITrackable*>,
			"a TrackingPointer must not silently decay back to a raw pointer");
	static_assert (!std::is_convertible_v<TrackingPointer<ITrackable>, bool>,
			"operator bool must remain explicit");

	void TrackingPointerTest::testTracksLiveObject ()
	{
		TrackableObject obj { 42 };
		ITrackable * const iface = &obj;

		const TrackingPointer<ITrackable> ptr { iface };

		QVERIFY (static_cast<bool> (ptr));
		QCOMPARE (ptr.UnsafeGet (), iface);
		QCOMPARE (ptr->GetValue (), 42);
		QCOMPARE ((*ptr).GetValue (), 42);
	}

	void TrackingPointerTest::testDetectsDestruction ()
	{
		auto obj = std::make_unique<TrackableObject> (5);
		const TrackingPointer<ITrackable> ptr { obj.get () };

		QVERIFY (ptr);

		obj.reset ();

		QVERIFY (!ptr);
	}

	void TrackingPointerTest::testDeadAccessThrows ()
	{
		auto obj = std::make_unique<TrackableObject> ();
		const TrackingPointer<ITrackable> ptr { obj.get () };
		obj.reset ();

		QVERIFY (!ptr);
		QVERIFY_THROWS_EXCEPTION (BadTrackingPointerAccess, ptr->GetValue ());
		QVERIFY_THROWS_EXCEPTION (BadTrackingPointerAccess, (*ptr).GetValue ());
	}

	void TrackingPointerTest::testNullIsInvalid ()
	{
		const TrackingPointer<ITrackable> defaulted;
		QVERIFY (!defaulted);
		QVERIFY (!defaulted.UnsafeGet ());

		ITrackable * const nullIface = nullptr;
		const TrackingPointer<ITrackable> fromNull { nullIface };
		QVERIFY (!fromNull);
		QVERIFY (!fromNull.UnsafeGet ());

		QVERIFY_THROWS_EXCEPTION (BadTrackingPointerAccess, fromNull->GetValue ());
	}

	void TrackingPointerTest::testCopyPropagatesTracking ()
	{
		auto obj = std::make_unique<TrackableObject> (7);

		TrackingPointer<ITrackable> copy;
		QVERIFY (!copy);

		{
			const TrackingPointer<ITrackable> original { obj.get () };
			copy = original;
		}

		QVERIFY (copy);
		QCOMPARE (copy->GetValue (), 7);

		obj.reset ();
		QVERIFY (!copy);
	}

	void TrackingPointerTest::testEqualityIsIdentity ()
	{
		TrackableObject first { 1 };
		TrackableObject second { 2 };

		const TrackingPointer<ITrackable> a { &first };
		const TrackingPointer<ITrackable> b { &first };
		const TrackingPointer<ITrackable> c { &second };

		QVERIFY (a == b);
		QVERIFY (a != c);

		ITrackable * const firstIface = &first;
		QVERIFY (a == firstIface);

		const TrackingPointer<ITrackable> empty;
		QVERIFY (empty == nullptr);
		QVERIFY (a != nullptr);
	}

	void TrackingPointerTest::testCustomGetter ()
	{
		auto anchor = std::make_unique<QObject> ();
		ExternallyBacked backed { anchor.get () };

		const TrackingPointer<ExternallyBacked, &ExternallyBacked::Anchor> viaMember { &backed };
		QVERIFY (viaMember);
		QCOMPARE (viaMember.UnsafeGet (), &backed);

		const TrackingPointer<ExternallyBacked, &AnchorOf> viaFreeFunc { &backed };
		QVERIFY (viaFreeFunc);

		anchor.reset ();
		QVERIFY (!viaMember);
		QVERIFY (!viaFreeFunc);
	}
}
