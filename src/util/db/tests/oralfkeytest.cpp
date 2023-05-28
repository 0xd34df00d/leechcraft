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

BOOST_FUSION_ADAPT_STRUCT (Student,
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

BOOST_FUSION_ADAPT_STRUCT (StudentInfo,
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

BOOST_FUSION_ADAPT_STRUCT (Lecturer,
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

BOOST_FUSION_ADAPT_STRUCT (Student2Lecturer,
		ID_,
		StudentID_,
		LecturerID_)

TOSTRING (Student2Lecturer)

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
}
}
