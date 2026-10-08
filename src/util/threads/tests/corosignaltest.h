/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaMethod>
#include <QObject>

namespace LC::Util
{
	class SignalEmitter : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;

		template<typename Sig>
		bool IsConnected (Sig signal) const
		{
			return isSignalConnected (QMetaMethod::fromSignal (signal));
		}
	signals:
		void NoArgs ();
		void OneArg (int);
		void RefArg (const QString&);
		void TwoArgs (int, QString);
		void Overloaded (int);
		void Overloaded (const QString&);
	};

	class CoroSignalTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testNoArgs ();
		void testOneArg ();
		void testTwoArgsYieldTuple ();
		void testRefArgYieldsOwnedValue ();
		void testOverloaded ();
		void testPrivateTagStripped ();
		void testBaseClassThroughDerived ();

		void testResumesAfterEmitReturns ();
		void testContinuationMayDestroySender ();
		void testFirstEmissionWins ();
		void testConnectedOnlyWhileSuspended ();
		void testManyAwaitersOneEmission ();
		void testReawaitSame ();

		void testContextDeathDisconnects ();
		void testEmittedFromAnotherThread ();
	};
}
