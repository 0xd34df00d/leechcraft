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

#include "xmlparsers.h"
#include <QUrl>
#include <QString>
#include <QXmlQuery>
#include <QDomDocument>
#include <QDomElement>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			RepoInfo ParseRepoInfo (const QUrl& url, const QString& data)
			{
				QXmlQuery query;
				query.setFocus (data);

				RepoInfo info (url);

				QString out;
				query.setQuery ("/repo/name/text()");
				if (!query.evaluateTo (&out))
					throw QObject::tr ("Could not get repo name.");
				info.SetName (out.simplified ());

				query.setQuery ("/repo/description/short/text()");
				if (!query.evaluateTo (&out))
					throw QObject::tr ("Could not get repo description.");
				info.SetShortDescr (out.simplified ());

				query.setQuery ("/repo/description/long/text()");
				if (!query.evaluateTo (&out))
					throw QObject::tr ("Could not get long repo description.");
				info.SetLongDescr (out.simplified ());

				MaintainerInfo maintInfo;
				query.setQuery ("/repo/maintainer/name/text()");
				if (!query.evaluateTo (&out))
					throw QObject::tr ("Could not get maintainer name.");
				maintInfo.Name_ = out.simplified ();

				query.setQuery ("/repo/maintainer/email/text()");
				if (!query.evaluateTo (&out))
					throw QObject::tr ("Could not get maintainer email.");
				maintInfo.Email_ = out.simplified ();

				info.SetMaintainer (maintInfo);

				QStringList components;
				query.setQuery ("/repo/components/component/text()");
				if (query.evaluateTo (&components))
					info.SetComponents (components);
				else if (query.evaluateTo (&out))
					info.SetComponents (QStringList (out));
				else
					throw QObject::tr ("Could not get components.");

				return info;
			}

			PackageShortInfoList ParseComponent (const QByteArray& data)
			{
				QDomDocument doc;
				QString msg;
				int line = 0;
				int column = 0;
				if (!doc.setContent (data, &msg, &line, &column))
				{
					qWarning () << Q_FUNC_INFO
							<< "erroneous document with msg"
							<< msg
							<< line
							<< column
							<< data;
					throw std::runtime_error ("Unable to parse component description.");
				}

				PackageShortInfoList infos;

				QDomElement root = doc.documentElement ();
				QDomElement package = root.firstChildElement ("package");
				while (!package.isNull ())
				{
					QStringList versionsList;
					QDomElement versions = package.firstChildElement ("versions");
					QDomElement version = versions.firstChildElement ("version");
					while (!version.isNull ())
					{
						versionsList << version.text ();

						version = version.nextSiblingElement ("version");
					}

					PackageShortInfo psi =
					{
						package.firstChildElement ("name").text (),
						versionsList
					};
					infos << psi;

					package = package.nextSiblingElement ("package");
				}

				return infos;
			}

			PackageInfo ParsePackage (const QByteArray& data,
					const QString& packageName, const QStringList& packageVersions)
			{
				QDomDocument doc;
				QString msg;
				int line = 0;
				int column = 0;
				if (!doc.setContent (data, &msg, &line, &column))
				{
					qWarning () << Q_FUNC_INFO
							<< "erroneous document with msg"
							<< msg
							<< line
							<< column
							<< data;
					throw std::runtime_error ("Unagle to parse package description.");
				}

				PackageInfo packageInfo;

				QDomElement package = doc.documentElement ();

				QString type = package.attribute ("type");
				if (type == "iconset")
					packageInfo.Type_ = PackageInfo::TIconset;
				else if (type == "translation")
					packageInfo.Type_ = PackageInfo::TTranslation;
				else
					packageInfo.Type_ = PackageInfo::TPlugin;

				packageInfo.Language_ = package.attribute ("language");
				packageInfo.Name_ = packageName;
				packageInfo.Versions_ = packageVersions;
				packageInfo.Description_ = package.firstChildElement ("description").text ();
				packageInfo.LongDescription_ = package.firstChildElement ("long").text ();

				QDomElement images = package.firstChildElement ("images");
				QDomElement imageNode = images.firstChildElement ("thumbnail");
				while (!imageNode.isNull ())
				{
					Image image =
					{
						Image::TThumbnail,
						imageNode.attribute ("url")
					};
					packageInfo.Images_ << image;
					imageNode = imageNode.nextSiblingElement ("thumbnail");
				}
				imageNode = images.firstChildElement ("screenshot");
				while (!imageNode.isNull ())
				{
					Image image =
					{
						Image::TScreenshot,
						imageNode.attribute ("url")
					};
					packageInfo.Images_ << image;
					imageNode = imageNode.nextSiblingElement ("screenshot");
				}
				packageInfo.IconURL_ = images.firstChildElement ("icon").attribute ("url");

				QDomElement tags = package.firstChildElement ("tags");
				QDomElement tagNode = tags.firstChildElement ("tag");
				while (!tagNode.isNull ())
				{
					packageInfo.Tags_ << tagNode.text ();
					tagNode = tagNode.nextSiblingElement ("tag");
				}

				QDomElement maintNode = package.firstChildElement ("maintainer");
				packageInfo.MaintName_ = maintNode.firstChildElement ("name").text ();
				packageInfo.MaintEmail_ = maintNode.firstChildElement ("email").text ();

				QDomElement depends = package.firstChildElement ("depends");
				QDomElement dependNode = depends.firstChildElement ("depend");
				while (!dependNode.isNull ())
				{
					Dependency dep;
					if (dependNode.attribute ("type") == "depends" ||
							!dependNode.hasAttribute ("type"))
						dep.Type_ = Dependency::TRequires;
					else
						dep.Type_ = Dependency::TProvides;
					dep.Name_ = dependNode.attribute ("name");
					dep.Version_ = dependNode.attribute ("version");

					packageInfo.Deps_ [dependNode.attribute ("thisVersion")] << dep;

					dependNode = dependNode.nextSiblingElement ("depend");
				}

				return packageInfo;
			}
		}
	}
}
