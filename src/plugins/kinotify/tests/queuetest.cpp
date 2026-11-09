/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "queuetest.h"
#include <QApplication>
#include <QPixmap>
#include <QStandardPaths>
#include <QTest>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <interfaces/entityconstants.h>
#include <interfaces/an/entityfields.h>
#include "../kinotify.h"
#include "../kinotifywidget.h"
#include "../xmlsettingsmanager.h"

QTEST_MAIN (LC::Kinotify::QueueTest)

namespace LC::Kinotify
{
	// platform mock
	bool IsCurrentWindowFullScreen ()
	{
		return false;
	}

	namespace
	{
		QtMessageHandler PrevHandler = nullptr;

		void DropOffscreenUnsupportedNoise (QtMsgType type, const QMessageLogContext& context, const QString& message)
		{
			if (!message.startsWith ("This plugin does not support"_ql))
				PrevHandler (type, context, message);
		}

		const auto A = "A"_qs;
		const auto A2 = "A2"_qs;
		const auto B = "B"_qs;
		const auto B2 = "B2"_qs;

		Entity MakeTestNotification (const QString& eventId, const QString& body)
		{
			auto e = Util::MakeNotification ("Kinotify"_qs, body, Priority::Info);
			e.Additional_ [AN::EF::SenderID] = "org.LeechCraft.Kinotify.Test"_qs;
			e.Additional_ [AN::EF::EventID] = eventId;
			e.Additional_ [EF::NotificationTimeout] = 50;
			// avoid depending on the icon theme manager and (transitively) the core proxy
			e.Additional_ [EF::NotificationPixmap] = QVariant::fromValue (QPixmap { 1, 1 });
			return e;
		}

		QList<KinotifyWidget*> Notifications ()
		{
			QList<KinotifyWidget*> result;
			for (const auto widget : QApplication::topLevelWidgets ())
				if (const auto notification = qobject_cast<KinotifyWidget*> (widget))
					result << notification;
			return result;
		}

		QStringList Bodies (const QList<KinotifyWidget*>& notifications)
		{
			QStringList result;
			for (const auto notification : notifications)
				result << notification->GetBody ();
			result.sort ();
			return result;
		}

		QStringList Alive ()
		{
			return Bodies (Notifications ());
		}

		QStringList Shown ()
		{
			auto notifications = Notifications ();
			notifications.removeIf ([] (const KinotifyWidget *notification) { return !notification->isVisible (); });
			return Bodies (notifications);
		}
	}

	void QueueTest::initMain ()
	{
		qputenv ("QT_QPA_PLATFORM", "offscreen");
		QStandardPaths::setTestModeEnabled (true);
	}

	void QueueTest::initTestCase ()
	{
		PrevHandler = qInstallMessageHandler (DropOffscreenUnsupportedNoise);
		// avoid depending on the root windows manager
		XmlSettingsManager::Instance ().setProperty ("FollowMouse", true);
	}

	void QueueTest::cleanup ()
	{
		qDeleteAll (Notifications ());
	}

	void QueueTest::testShowsTheNextOneWhenTheDisplayedOneExpires ()
	{
		Plugin plugin;
		plugin.Handle (MakeTestNotification ("a"_qs, A));
		plugin.Handle (MakeTestNotification ("b"_qs, B));
		QCOMPARE (Alive (), (QStringList { A, B }));
		QCOMPARE (Shown (), QStringList { A });

		QTRY_COMPARE (Alive (), QStringList { B });
		QCOMPARE (Shown (), QStringList { B });

		QTRY_VERIFY (Alive ().isEmpty ());
	}

	void QueueTest::testReplacingTheDisplayedOneShowsTheReplacement ()
	{
		Plugin plugin;
		plugin.Handle (MakeTestNotification ("a"_qs, A));
		plugin.Handle (MakeTestNotification ("a"_qs, A2));

		QTRY_COMPARE (Alive (), QStringList { A2 });
		QCOMPARE (Shown (), QStringList { A2 });

		QTRY_VERIFY (Alive ().isEmpty ());
	}

	void QueueTest::testReplacingAQueuedOneKeepsTheDisplayedOne ()
	{
		Plugin plugin;
		plugin.Handle (MakeTestNotification ("a"_qs, A));
		plugin.Handle (MakeTestNotification ("b"_qs, B));
		plugin.Handle (MakeTestNotification ("b"_qs, B2));

		QTRY_COMPARE (Alive (), (QStringList { A, B2 }));
		QCOMPARE (Shown (), QStringList { A });

		QTRY_COMPARE (Alive (), QStringList { B2 });
		QCOMPARE (Shown (), QStringList { B2 });

		QTRY_VERIFY (Alive ().isEmpty ());
	}
}
