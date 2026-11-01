/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entryidopstest.h"
#include <QRegularExpression>
#include <QTest>
#include <QVariant>
#include <util/azoth/entryidops.h>
#include <util/azoth/util.h>

QTEST_GUILESS_MAIN (LC::Azoth::EntryIdOpsTest)

namespace LC::Azoth
{
	namespace
	{
		template<typename Id>
		concept DeserializableFromCString = requires { Deserialize<Id> ("juliet@capulet.lit"); };

		static_assert (!DeserializableFromCString<EntryStrongestId>);

		template<AnyId Id>
		void CheckVariantRoundTrip (const Id& id)
		{
			const auto& fromVariant = IdFromVariant<Id> (QVariant::fromValue (id));
			QVERIFY (fromVariant);
			QCOMPARE (*fromVariant, id);
		}

		template<AnyEntryId Id>
		void CheckRoundTrip (const Id& id)
		{
			const auto& fromBytes = Deserialize<Id> (SerializeBytes (id));
			QVERIFY (fromBytes);
			QCOMPARE (*fromBytes, id);

			const auto& fromString = Deserialize<Id> (SerializeString (id));
			QVERIFY (fromString);
			QCOMPARE (*fromString, id);

			CheckVariantRoundTrip (id);
		}

		template<AnyEntryId Id>
		void CheckRejected (const auto& serialized)
		{
			QTest::ignoreMessage (QtWarningMsg, QRegularExpression { ".*" });
			QVERIFY (!Deserialize<Id> (serialized));
		}
	}

	void EntryIdOpsTest::testRoundTrip ()
	{
		QFETCH (QString, account);
		QFETCH (QString, entry);

		const auto accountId = AccountId::FromString (account);
		const auto persistent = EntryPersistentId::FromString (entry);
		const auto conventional = EntryConventionalId::FromString (entry);

		CheckVariantRoundTrip (accountId);
		CheckRoundTrip (persistent);
		CheckRoundTrip (conventional);
		CheckRoundTrip (GlobalPersistentId { accountId, persistent });
		CheckRoundTrip (GlobalConventionalId { accountId, conventional });
		CheckRoundTrip (EntryStrongestId { persistent });
		CheckRoundTrip (EntryStrongestId { conventional });
		CheckRoundTrip (GlobalStrongestId { GlobalPersistentId { accountId, persistent } });
		CheckRoundTrip (GlobalStrongestId { GlobalConventionalId { accountId, conventional } });
	}

	void EntryIdOpsTest::testRoundTrip_data ()
	{
		QTest::addColumn<QString> ("account");
		QTest::addColumn<QString> ("entry");

		const auto xmpp = "Xoox.Gloox.XMPP_romeo@montague.lit";
		QTest::newRow ("xmpp contact") << xmpp << "juliet@capulet.lit";
		QTest::newRow ("xmpp room participant") << xmpp << "capulet-ball@conference.capulet.lit/Juliet Capulet 🌙";
		QTest::newRow ("xmpp idn") << "Xoox.Gloox.XMPP_ромео@монтекки.рф" << "джульетта@капулетти.рф";

		const auto irc = "Acetamide.IRC.1760636400";
		QTest::newRow ("irc channel") << irc << "irc.libera.chat:6697/#leechcraft@irc.libera.chat";
		QTest::newRow ("irc channel participant") << irc << "Juliet[m]@irc.libera.chat:6697/#leechcraft@irc.libera.chat";
		QTest::newRow ("irc private chat") << irc << "Juliet_@irc.libera.chat:6697";

		QTest::newRow ("tox contact") << "{3f2b1e0c-9a8d-4c7b-b6a5-0e1d2c3b4a59}" << "C69C4F9E0BE5B9ECF24D36B0C8E4AE4D5B5EB9D69B6E0F7A1C2D3E4F50617283";

		const auto matrix = "Matrix_@romeo:montague.lit:8448";
		QTest::newRow ("matrix user") << matrix << "@juliet:capulet.lit:8448";
		QTest::newRow ("matrix room") << matrix << "!HuhSxqaBfzQKeXLfBd:capulet.lit";

		QTest::newRow ("digit-only account") << "42" << "7:11";
		QTest::newRow ("non-BMP account") << "ромео🎭" << "🌙";
		QTest::newRow ("colons at the edges") << ":acc:" << ":entry:";
	}

	void EntryIdOpsTest::testPersistedStringFormat ()
	{
		const auto romeo = AccountId::FromString ("romeo");
		const auto persistent = EntryPersistentId::FromString ("juliet@capulet.lit");
		const auto conventional = EntryConventionalId::FromString ("juliet@capulet.lit");

		QCOMPARE (SerializeString (persistent), "juliet@capulet.lit");
		QCOMPARE (SerializeString (GlobalPersistentId { romeo, persistent }), "5:romeo:juliet@capulet.lit");
		QCOMPARE (SerializeString (EntryStrongestId { persistent }), "P:juliet@capulet.lit");
		QCOMPARE (SerializeString (EntryStrongestId { conventional }), "C:juliet@capulet.lit");
		QCOMPARE (SerializeString (GlobalStrongestId { GlobalPersistentId { romeo, persistent } }), "P:5:romeo:juliet@capulet.lit");
	}

	void EntryIdOpsTest::testMalformedStrings ()
	{
		CheckRejected<EntryStrongestId> (u"");
		CheckRejected<EntryStrongestId> (u"P");
		CheckRejected<EntryStrongestId> (u"Pjuliet");
		CheckRejected<EntryStrongestId> (u"X:juliet");

		CheckRejected<GlobalPersistentId> (u"romeo");
		CheckRejected<GlobalPersistentId> (u"romeo:juliet");
		CheckRejected<GlobalPersistentId> (u"-1:romeo:juliet");
		CheckRejected<GlobalPersistentId> (u"4:romeo:juliet");
		CheckRejected<GlobalPersistentId> (u"5:romeo");
		CheckRejected<GlobalPersistentId> (u"99:romeo:juliet");

		CheckRejected<GlobalStrongestId> (u"P:romeo:juliet");
	}

	void EntryIdOpsTest::testMalformedBytes ()
	{
		const GlobalPersistentId globalId { AccountId::FromString ("romeo"), EntryPersistentId::FromString ("juliet@capulet.lit") };
		const GlobalStrongestId id { globalId };

		CheckRejected<GlobalStrongestId> (QByteArray {});

		auto truncated = SerializeBytes (id);
		truncated.chop (1);
		CheckRejected<GlobalStrongestId> (truncated);

		CheckRejected<GlobalStrongestId> (SerializeBytes (id) + "trailing");

		const auto& nested = SerializeBytes (id);
		const auto& inner = SerializeBytes (globalId);
		QVERIFY (nested.endsWith (inner));

		auto badKind = nested;
		badKind [nested.size () - inner.size () - 1] = 7;
		CheckRejected<GlobalStrongestId> (badKind);
	}

	void EntryIdOpsTest::testVariantTypeMismatch ()
	{
		const auto romeo = AccountId::FromString ("romeo");
		const auto persistent = EntryPersistentId::FromString ("juliet@capulet.lit");

		QVERIFY (!IdFromVariant<EntryPersistentId> (QVariant {}));
		QVERIFY (!IdFromVariant<EntryPersistentId> (QVariant { persistent.ToString () }));
		QVERIFY (!IdFromVariant<EntryPersistentId> (QVariant { SerializeBytes (persistent) }));
		QVERIFY (!IdFromVariant<EntryConventionalId> (QVariant::fromValue (persistent)));
		QVERIFY (!IdFromVariant<EntryStrongestId> (QVariant::fromValue (persistent)));
		QVERIFY (!IdFromVariant<AccountId> (QVariant { romeo.ToString () }));
	}
}
