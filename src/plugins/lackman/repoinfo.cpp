/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "repoinfo.h"
#include <QtDebug>
#include "versionverifier.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			RepoInfo::RepoInfo ()
			{
			}

			RepoInfo::RepoInfo (const QUrl& url)
			: URL_ (url)
			{
			}

			RepoInfo::RepoInfo (const QUrl& url, const QString& name,
					const QString& shortDescr, const QStringList& components)
			: URL_ (url)
			, Name_ (name)
			, ShortDescr_ (shortDescr)
			, Components_ (components)
			{
			}

			const QUrl& RepoInfo::GetUrl () const
			{
				return URL_;
			}

			void RepoInfo::SetUrl (const QUrl& url)
			{
				URL_ = url;
			}

			const QString& RepoInfo::GetName () const
			{
				return Name_;
			}

			void RepoInfo::SetName (const QString& name)
			{
				Name_ = name;
			}

			const QString& RepoInfo::GetShortDescr () const
			{
				return ShortDescr_;
			}

			void RepoInfo::SetShortDescr (const QString& descr)
			{
				ShortDescr_ = descr;
			}

			const QString& RepoInfo::GetLongDescr () const
			{
				return LongDescr_;
			}

			void RepoInfo::SetLongDescr (const QString& descr)
			{
				LongDescr_ = descr;
			}

			const MaintainerInfo& RepoInfo::GetMaintainer () const
			{
				return Maintainer_;
			}

			void RepoInfo::SetMaintainer (const MaintainerInfo& maint)
			{
				Maintainer_ = maint;
			}

			const QStringList& RepoInfo::GetComponents () const
			{
				return Components_;
			}

			void RepoInfo::SetComponents (const QStringList& components)
			{
				Components_.clear ();
				Q_FOREACH (const QString& c, components)
					Components_ << c.simplified ();
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
						<< "; dependencies:";
				Q_FOREACH (QString version, Deps_.keys ())
					Q_FOREACH (const Dependency& d, Deps_ [version])
						qDebug () << "\t" << version << d.Type_ << d.Name_ << d.Version_;
				if (Images_.size ())
				{
					qDebug () << "; images:";
					Q_FOREACH (const Image& img, Images_)
						qDebug () << "\t" << img.Type_ << img.URL_;
				}
			}

			bool operator== (const ListPackageInfo& lpi1, const ListPackageInfo& lpi2)
			{
				return lpi1.PackageID_ == lpi2.PackageID_;
			}

			bool IsVersionLess (const QString& lver, const QString& rver)
			{
				VersionVerifier vv;
				return vv.CompareVersions (lver, rver) < 0;
			}

			uint qHash (const Dependency& dep)
			{
				return qHash (QString::number (dep.Type_) + dep.Name_ + dep.Version_);
			}
		}
	}
}
