/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fixedstringfilterproxymodeltest.h"
#include <functional>
#include <QStringListModel>
#include <QtTest>
#include <util/sll/qtutil.h>
#include "fixedstringfilterproxymodel.h"

QTEST_GUILESS_MAIN (LC::Util::FixedStringFilterProxyModelTest)

namespace LC::Util
{
	namespace
	{
		QStringList Criticals;

		class CriticalsCollector
		{
			QtMessageHandler Prev_;
		public:
			CriticalsCollector ()
			: Prev_ { qInstallMessageHandler (&Handle) }
			{
				Criticals.clear ();
			}

			~CriticalsCollector ()
			{
				qInstallMessageHandler (Prev_);
			}

			CriticalsCollector (const CriticalsCollector&) = delete;
			CriticalsCollector& operator= (const CriticalsCollector&) = delete;
		private:
			static void Handle (QtMsgType type, const QMessageLogContext&, const QString& msg)
			{
				if (type == QtCriticalMsg)
					Criticals << msg;
			}
		};

		QStringList Rows (const QAbstractItemModel& model)
		{
			QStringList result;
			for (int i = 0; i < model.rowCount (); ++i)
				result << model.index (i, 0).data ().toString ();
			return result;
		}
	}

	void FixedStringFilterProxyModelTest::testFiltering ()
	{
		QStringListModel source { { u"Pink Floyd"_qs, u"AC/DC"_qs, u"Кино"_qs, u"Beatles"_qs } };
		FixedStringFilterProxyModel model;
		model.setSourceModel (&source);

		model.SetFilterString (u"k f"_qs);
		QCOMPARE (Rows (model), (QStringList { u"Pink Floyd"_qs }));

		model.SetFilterString (u"/"_qs);
		QCOMPARE (Rows (model), (QStringList { u"AC/DC"_qs }));

		model.SetFilterString (u"кИн"_qs);
		QCOMPARE (Rows (model), (QStringList { u"Кино"_qs }));

		model.SetCaseSensitivity (Qt::CaseSensitive);
		QCOMPARE (Rows (model), QStringList {});
	}

	void FixedStringFilterProxyModelTest::testOwnSettersAreQuiet ()
	{
		QStringListModel source { { u"a"_qs, u"b"_qs } };

		CriticalsCollector collector;

		FixedStringFilterProxyModel model;
		model.setSourceModel (&source);
		model.SetFilterString (u"a"_qs);
		model.SetCaseSensitivity (Qt::CaseSensitive);
		model.SetFilterRoles ({ Qt::DisplayRole, Qt::EditRole });
		model.SetFilterColumns ({ 0 });
		model.setRecursiveFilteringEnabled (false);
		model.setAutoAcceptChildRows (true);
		model.setSortRole (Qt::EditRole);
		model.sort (0);

		QCOMPARE (Criticals, QStringList {});
	}

	void FixedStringFilterProxyModelTest::testQtSettersAreReported ()
	{
		using Setter_f = std::function<void (FixedStringFilterProxyModel&)>;
		const std::initializer_list<std::tuple<const char*, Setter_f, const char*>> cases
		{
			{
				"fixed string via base",
				[] (QSortFilterProxyModel& m) { m.setFilterFixedString (u"a"_qs); },
				"SetFilterString ()"
			},
			{
				"fixed string by name",
				[] (QObject& m) { QMetaObject::invokeMethod (&m, "setFilterFixedString", u"a"_qs); },
				"SetFilterString ()"
			},
			{
				"wildcard",
				[] (QSortFilterProxyModel& m) { m.setFilterWildcard (u"a*"_qs); },
				"SetFilterString ()"
			},
			{
				"regex",
				[] (QSortFilterProxyModel& m) { m.setFilterRegularExpression (u"a"_qs); },
				"SetFilterString ()"
			},
			{
				"case sensitivity",
				[] (QSortFilterProxyModel& m) { m.setFilterCaseSensitivity (Qt::CaseInsensitive); },
				"SetCaseSensitivity ()"
			},
			{
				"key column",
				[] (QSortFilterProxyModel& m) { m.setFilterKeyColumn (1); },
				"SetFilterColumns ()"
			},
			{
				"role",
				[] (QSortFilterProxyModel& m) { m.setFilterRole (Qt::UserRole); },
				"SetFilterRoles ()"
			},
		};

		for (const auto& [name, setter, replacement] : cases)
		{
			FixedStringFilterProxyModel model;

			CriticalsCollector collector;
			setter (model);

			QVERIFY2 (Criticals.size () == 1, name);
			QVERIFY2 (Criticals.front ().contains (QLatin1StringView { replacement }), name);
		}
	}
}
