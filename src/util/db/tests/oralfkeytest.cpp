/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "oralfkeytest.h"
#include "common.h"

QTEST_GUILESS_MAIN (LeechCraft::Util::OralFKeyTest)

struct Student
{
	lco::PKey<int> ID_;
	QString Name_;

	static QString ClassName ()
	{
		return "Student";
	}

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

	static QString ClassName ()
	{
		return "StudentInfo";
	}

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

	static QString ClassName ()
	{
		return "Lecturer";
	}

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

	static QString ClassName ()
	{
		return "Student2Lecturer";
	}

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

namespace LeechCraft
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
