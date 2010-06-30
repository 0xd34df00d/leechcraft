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
					TProvides,
					TRequires
				} Type_;
				QString Name_;
			};

			struct PackageVersionInfo
			{
				enum Type
				{
					TTranslation,
					TPlugin,
					TIconset
				} Type_;
				QString Language_;
				QString Description_;
				QString LongDescription_;
				QStringList Tags_;
			};

			struct PackageInfo : PackageShortInfo
			{
				typedef QMap<QString, PackageVersionInfo> Version2Info_t;
				Version2Info_t Infos_;
			};
		}
	}
}

Q_DECLARE_METATYPE (LeechCraft::Plugins::LackMan::RepoInfo);
Q_DECLARE_METATYPE (LeechCraft::Plugins::LackMan::PackageShortInfo);
Q_DECLARE_METATYPE (LeechCraft::Plugins::LackMan::PackageShortInfoList);

#endif
