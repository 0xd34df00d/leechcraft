/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_REPOINFO_H
#define PLUGINS_LACKMAN_REPOINFO_H
#include <functional>
#include <QMetaType>
#include <QStringList>
#include <QUrl>
#include <QMap>

namespace LC
{
namespace LackMan
{
	struct MaintainerInfo
	{
		QString Name_;
		QString Email_;
	};

	using MaintainerInfoList = QList<MaintainerInfo>;

	struct RepoInfo
	{
		QUrl URL_;
		QString Name_;
		QString ShortDescr_;
		QString LongDescr_;

		MaintainerInfo Maintainer_;
		QStringList Components_;

		bool IsValid () const;
	};

	struct PackageShortInfo
	{
		QString Name_;
		QStringList Versions_;
		QMap<QString, QString> VersionArchivers_;
	};

	using PackageShortInfoList = QList<PackageShortInfo>;

	struct Dependency
	{
		enum Type
		{
			TRequires,
			TProvides,
			TMAX
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

	using DependencyList = QList<Dependency>;

	bool operator== (const Dependency&, const Dependency&);

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
			/** @brief Package with a plugin for LeechCraft.
				*
				* Contents of packages of this type would be
				* installed into ~/.leechcraft/plugins/scriptable/$language
				*/
			TPlugin,

			/** @brief Translation (or a set of translations).
				*
				* Contents of packages of this type would be
				* installed into ~/.leechcraft/translations
				*/
			TTranslation,

			/** @brief Iconset package.
				*
				* Contents of packages of this type would be
				* installed into ~/.leechcraft/icons
				*/
			TIconset,

			/** @brief Plain data package.
				*
				* Use this if nothing else is appropriate.
				*
				* Contents of packages of this type would be
				* installed into ~/.leechcraft/data
				*/
			TData,

			/** @brief Theme package.
				*
				* Contents of packages of this type would be
				* installed into ~/.leechcraft/data
				*/
			TTheme,

			/** @brief Quark package.
			 */
			TQuark
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
		QMap<QString, qint64> PackageSizes_;

		void Dump () const;
	};

	/** This contains those and only those fields which are
		* displayed in the Packages list.
		*/
	struct ListPackageInfo
	{
		int PackageID_;
		QString Name_;
		QString Version_;
		QString ShortDescription_;
		QString LongDescription_;
		PackageInfo::Type Type_;
		QString Language_;
		QUrl IconURL_;
		QStringList Tags_;
		bool HasNewVersion_;
		bool IsInstalled_;
	};

	bool operator== (const ListPackageInfo&, const ListPackageInfo&);

	using Comparator_t = std::function<bool (const QString&, const QString&)>;

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

	using InstalledDependencyInfoList = QList<InstalledDependencyInfo>;

	uint qHash (const Dependency&);
}
}

Q_DECLARE_METATYPE (LC::LackMan::RepoInfo)
Q_DECLARE_METATYPE (LC::LackMan::PackageShortInfo)
Q_DECLARE_METATYPE (LC::LackMan::PackageShortInfoList)
Q_DECLARE_METATYPE (LC::LackMan::PackageInfo)

#endif
