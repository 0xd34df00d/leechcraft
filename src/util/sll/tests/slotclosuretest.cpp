/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "slotclosuretest.h"
#include <QtTest>
#include "slotclosure.h"

QTEST_MAIN (LC::Util::SlotClosureTest)

namespace LC
{
namespace Util
{
	void DummyObject::EmitSignal ()
	{
		emit someSignal ();
	}

	void SlotClosureTest::testDeleteLater ()
	{
		DummyObject obj;

		bool hasRun = false;
		const auto closure = new SlotClosure<DeleteLaterPolicy>
		{
			[&hasRun]
			{
				hasRun = true;
			},
			&obj,
			SIGNAL (someSignal ()),
			nullptr
		};

		obj.EmitSignal ();

		const QPointer<QObject> closurePtr { closure };

		QCOMPARE (hasRun, true);
		QCOMPARE (closurePtr.isNull (), false);

		QCoreApplication::sendPostedEvents (nullptr, QEvent::DeferredDelete);

		QCOMPARE (closurePtr.isNull (), true);
	}

	void SlotClosureTest::testNoDelete ()
	{
		DummyObject obj;

		bool hasRun = false;
		const auto closure = new SlotClosure<NoDeletePolicy>
		{
			[&hasRun]
			{
				hasRun = true;
			},
			&obj,
			SIGNAL (someSignal ()),
			nullptr
		};

		obj.EmitSignal ();

		const QPointer<QObject> closurePtr { closure };

		QCOMPARE (hasRun, true);
		QCOMPARE (closurePtr.isNull (), false);

		QCoreApplication::sendPostedEvents (nullptr, QEvent::DeferredDelete);

		QCOMPARE (closurePtr.isNull (), false);
	}

	void SlotClosureTest::testChoiceDelete ()
	{
		DummyObject obj;

		bool hasRun = false;
		const auto closure = new SlotClosure<ChoiceDeletePolicy>
		{
			[&hasRun]
			{
				if (hasRun)
					return ChoiceDeletePolicy::Delete::Yes;

				hasRun = true;
				return ChoiceDeletePolicy::Delete::No;
			},
			&obj,
			SIGNAL (someSignal ()),
			nullptr
		};
		const QPointer<QObject> closurePtr { closure };

		obj.EmitSignal ();

		QCOMPARE (hasRun, true);
		QCOMPARE (closurePtr.isNull (), false);

		QCoreApplication::sendPostedEvents (nullptr, QEvent::DeferredDelete);

		QCOMPARE (closurePtr.isNull (), false);

		obj.EmitSignal ();
		QCoreApplication::sendPostedEvents (nullptr, QEvent::DeferredDelete);

		QCOMPARE (closurePtr.isNull (), true);
	}
}
}
