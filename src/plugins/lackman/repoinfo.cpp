/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "repoinfo.h"
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC
{
namespace LackMan
{
	bool RepoInfo::IsValid () const
	{
		return !Name_.isEmpty () && !Components_.isEmpty ();
	}

	bool operator== (const Dependency& dep1, const Dependency& dep2)
	{
		return dep1.Type_ == dep2.Type_ &&
				dep1.Name_ == dep2.Name_ &&
				dep1.Version_ == dep2.Version_;
	}

	void PackageInfo::Dump () const
	{
		qDebug () << "Package name: " << Name_
				<< "; versions:" << Versions_
				<< "; type:" << Type_
				<< "; language:" << Language_
				<< "; description:" << Description_
				<< "; long descr:" << LongDescription_
				<< "; tags:" << Tags_
				<< "; maintainer:" << MaintName_ << MaintEmail_
				<< "; icon:" << IconURL_
				<< "; package sizes:" << PackageSizes_
				<< "; dependencies:";

		for (auto [version, deps] : Util::Stlize (Deps_))
			for (const auto& d : deps)
				qDebug () << "\t" << version << d.Type_ << d.Name_ << d.Version_;

		if (!Images_.isEmpty ())
		{
			qDebug () << "; images:";
			for (const auto& img : Images_)
				qDebug () << "\t" << img.Type_ << img.URL_;
		}
	}

	bool operator== (const ListPackageInfo& lpi1, const ListPackageInfo& lpi2)
	{
		return lpi1.PackageID_ == lpi2.PackageID_;
	}

	uint qHash (const Dependency& dep)
	{
		return qHash (QString::number (dep.Type_) + dep.Name_ + dep.Version_);
	}
}
}
