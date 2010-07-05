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

#ifndef PLUGINS_LACKMAN_REPOINFO_H
#define PLUGINS_LACKMAN_REPOINFO_H
#include <boost/function.hpp>
#include <QMetaType>
#include <QStringList>
#include <QUrl>
#include <QMap>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			struct MaintainerInfo
			{
				QString Name_;
				QString Email_;
			};

			typedef QList<MaintainerInfo> MaintainerInfoList;

			class RepoInfo
			{
				QUrl URL_;
				QString Name_;
				QString ShortDescr_;
				QString LongDescr_;

				MaintainerInfo Maintainer_;
				QStringList Components_;
			public:
				explicit RepoInfo ();
				explicit RepoInfo (const QUrl&);
				RepoInfo (const QUrl& url, const QString& name,
						const QString& shortDescr, const QStringList& components);

				const QUrl& GetUrl () const;
				void SetUrl (const QUrl&);
				const QString& GetName () const;
				void SetName (const QString&);
				const QString& GetShortDescr () const;
				void SetShortDescr (const QString&);
				const QString& GetLongDescr () const;
				void SetLongDescr (const QString&);
				const MaintainerInfo& GetMaintainer () const;
				void SetMaintainer (const MaintainerInfo&);
				const QStringList& GetComponents () const;
				void SetComponents (const QStringList&);
			};

			struct PackageShortInfo
			{
				QString Name_;
				QStringList Versions_;
			};

			typedef QList<PackageShortInfo> PackageShortInfoList;

			struct Dependency
			{
				enum Type
				{
					TRequires,
					TProvides
				} Type_;

				/** Dependency version `relation` candidate version.
				 */
				enum Relation
				{
					G, //!< G Dependency version should be greater than candidate's.
					E, //!< E Dependency version should be equal to candidate's.
					L, //!< L Dependency version should be less than candidate's.
					GE,//!< GE Dependency version should be greater or equal to candidate's.
					LE //!< LE Dependency version should be less or equal to candidate's.
				};

				QString Name_;
				QString Version_;
			};

			typedef QList<Dependency> DependencyList;

			struct Image
			{
				enum Type
				{
					TScreenshot,
					TThumbnail
				} Type_;
				QString URL_;
			};

			struct PackageInfo : PackageShortInfo
			{
				enum Type
				{
					TPlugin,
					TTranslation,
					TIconset
				} Type_;
				QString Language_;
				QString Description_;
				QString LongDescription_;
				QStringList Tags_;
				QMap<QString, DependencyList> Deps_;
				QString MaintName_;
				QString MaintEmail_;
				QUrl IconURL_;
				QList<Image> Images_;

				void Dump () const;
			};

			/** This contains those and only those fields which are
			 * displayed in the Pacakges list.
			 */
			struct ListPackageInfo
			{
				int PackageID_;
				QString Name_;
				QString Version_;
				QString ShortDescription_;
				QString LongDescription_;
				PackageInfo::Type Type_;
				QUrl IconURL_;
				QStringList Tags_;
				bool HasNewVersion_;
				bool IsInstalled_;
			};

			/** Some kind of operator< for version strings.
			 */
			bool IsVersionLess (const QString&, const QString&);

			typedef boost::function<bool (const QString&, const QString&)> Comparator_t;

			extern QMap<Dependency::Relation, Comparator_t> Relation2comparator;

			/** Describes an installed dependency. Installed dependency may
			 * come either from system package manager of be installed
			 * by LackMan. Source enum is used to distinguish between
			 * these two cases.
			 */
			struct InstalledDependencyInfo
			{
				Dependency Dep_;

				/** Whether package was installed via standard system
				 * means like system package manager or via LackMan.
				 */
				enum Source
				{
					SSystem,//!< SSystem Package came from system.
					SLackMan//!< SLackMan Package came via LackMan.
				} Source_;
			};

			typedef QList<InstalledDependencyInfo> InstalledDependencyInfoList;
		}
	}
}

Q_DECLARE_METATYPE (LeechCraft::Plugins::LackMan::RepoInfo);
Q_DECLARE_METATYPE (LeechCraft::Plugins::LackMan::PackageShortInfo);
Q_DECLARE_METATYPE (LeechCraft::Plugins::LackMan::PackageShortInfoList);
Q_DECLARE_METATYPE (LeechCraft::Plugins::LackMan::PackageInfo);

#endif
