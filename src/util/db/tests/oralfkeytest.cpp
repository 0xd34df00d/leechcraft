/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "oralfkeytest.h"
#include "common.h"

QTEST_GUILESS_MAIN (LC::Util::OralFKeyTest)

using LC::operator""_ct;

struct Student
{
	lco::PKey<int> ID_;
	QString Name_;

	constexpr static auto ClassName = "Student"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Name_);
	}
};

ORAL_ADAPT_STRUCT (Student,
		ID_,
		Name_)

TOSTRING (Student)

struct StudentInfo
{
	lco::PKey<int> ID_;
	lco::References<&Student::ID_> StudentID_;
	int Age_;
	int Year_;

	constexpr static auto ClassName = "StudentInfo"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, StudentID_, Age_, Year_);
	}
};

ORAL_ADAPT_STRUCT (StudentInfo,
		ID_,
		StudentID_,
		Age_,
		Year_)

TOSTRING (StudentInfo)

struct Lecturer
{
	lco::PKey<int> ID_;
	QString Name_;

	constexpr static auto ClassName = "Lecturer"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Name_);
	}
};

ORAL_ADAPT_STRUCT (Lecturer,
		ID_,
		Name_)

TOSTRING (Lecturer)

struct Student2Lecturer
{
	lco::PKey<int> ID_;
	lco::References<&Student::ID_> StudentID_;
	lco::References<&Lecturer::ID_> LecturerID_;

	constexpr static auto ClassName = "Student2Lecturer"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, StudentID_, LecturerID_);
	}
};

ORAL_ADAPT_STRUCT (Student2Lecturer,
		ID_,
		StudentID_,
		LecturerID_)

TOSTRING (Student2Lecturer)

struct StudentExtra
{
	lco::PKey<lco::References<&Student::ID_>> Id_ {};
	lco::NotNull<QString> ExtraName_;

	constexpr static auto ClassName = "StudentExtra"_ct;

	auto AsTuple () const
	{
		return std::tie (Id_, ExtraName_);
	}
};

ORAL_ADAPT_STRUCT (StudentExtra,
		Id_,
		ExtraName_)

TOSTRING (StudentExtra)

namespace LC
{
namespace Util
{
	void OralFKeyTest::testBasicFKeys ()
	{
		auto db = MakeDatabase ();

		auto student = Util::oral::AdaptPtr<Student, OralFactory> (db);
		auto studentInfo = Util::oral::AdaptPtr<StudentInfo, OralFactory> (db);

		QList<QPair<Student, StudentInfo>> list
		{
			{ { 0, "Student 1" }, { 0, 0, 18, 1 } },
			{ { 0, "Student 2" }, { 0, 0, 19, 1 } },
			{ { 0, "Student 3" }, { 0, 0, 19, 2 } },
		};

		for (auto& [stud, info] : list)
		{
			student->Insert (stud);
			info.StudentID_ = stud.ID_;
			studentInfo->Insert (info);
		}

		namespace sph = oral::sph;

		const auto& selected = student->Select (sph::f<&Student::ID_> == sph::f<&StudentInfo::StudentID_> &&
				sph::f<&StudentInfo::Age_> > 18);
		const QList<Student> expected { list [1].first, list [2].first };
		QCOMPARE (selected, expected);
	}

	void OralFKeyTest::testPKeyReferencesInsert ()
	{
		auto db = MakeDatabase ();

		const auto student = lco::AdaptPtr<Student, OralFactory> (db);
		const auto extra = lco::AdaptPtr<StudentExtra, OralFactory> (db);

		const auto id = student->Insert ({ {}, "Student 1" });
		extra->Insert ({ .Id_ = id, .ExtraName_ = "foo" });

		const auto& selected = extra->Select ();
		QCOMPARE (selected, (QList { StudentExtra { 1, "foo" } }));
	}

	void OralFKeyTest::testPKeyReferencesUpsert ()
	{
		auto db = MakeDatabase ();

		const auto student = lco::AdaptPtr<Student, OralFactory> (db);
		const auto extra = lco::AdaptPtr<StudentExtra, OralFactory> (db);

		const auto id = student->Insert ({ {}, "Student 1" });
		extra->Insert ({ .Id_ = id, .ExtraName_ = "foo" });
		extra->Insert ({ .Id_ = id, .ExtraName_ = "bar" }, lco::InsertAction::Replace::Fields<&StudentExtra::ExtraName_>);

		const auto& selected = extra->Select ();
		QCOMPARE (selected, (QList { StudentExtra { 1, "bar" } }));
	}
}
}
