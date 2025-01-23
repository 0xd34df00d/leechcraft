/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlparsers.h"
#include <stdexcept>
#include <QObject>
#include <QUrl>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QtDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>

namespace LC
{
namespace LackMan
{
	RepoInfo ParseRepoInfo (const QUrl& url, const QString& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
			throw QObject::tr ("Could not parse repo data.");

		const auto& repo = doc.documentElement ().firstChildElement ("repo"_qs);
		const auto& descr = repo.firstChildElement ("description"_qs);
		const auto& maint = repo.firstChildElement ("maintainer"_qs);

		RepoInfo info
		{
			.URL_ = url,
			.Name_ = repo.firstChildElement ("name"_qs).text (),
			.ShortDescr_ = descr.firstChildElement ("short"_qs).text (),
			.LongDescr_ = descr.firstChildElement ("long"_qs).text (),
			.Maintainer_ =
				{
					.Name_ = maint.firstChildElement ("name"_qs).text (),
					.Email_ = maint.firstChildElement ("email"_qs).text (),
				},
		};

		for (const auto& component : Util::DomChildren (repo.firstChildElement ("components"_qs), "component"_qs))
			info.Components_ << component.text ();

		if (!info.IsValid ())
			throw QObject::tr ("Incomplete repo information.");

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
			QMap<QString, QString> archivers;

			QDomElement versions = package.firstChildElement ("versions");
			QDomElement version = versions.firstChildElement ("version");
			while (!version.isNull ())
			{
				const QString& txt = version.text ();
				versionsList << txt;
				archivers [txt] = version.attribute ("archiver", "gz");

				version = version.nextSiblingElement ("version");
			}

			PackageShortInfo psi =
			{
				package.firstChildElement ("name").text (),
				versionsList,
				archivers
			};
			infos << psi;

			package = package.nextSiblingElement ("package");
		}

		return infos;
	}

	namespace
	{
		QString MakeProperURL (const QString& urlStr, const QUrl& baseUrl)
		{
			QUrl testUrl (urlStr);
			if (testUrl.isValid () && !testUrl.isRelative ())
				return urlStr;

			QUrl r = baseUrl;
			r.setPath (r.path () + urlStr);
			return r.toEncoded ();
		}
	}

	PackageInfo ParsePackage (const QByteArray& data,
			const QUrl& baseUrl,
			const QString& packageName,
			const QStringList& packageVersions)
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
		else if (type == "plugin")
			packageInfo.Type_ = PackageInfo::TPlugin;
		else if (type == "theme")
			packageInfo.Type_ = PackageInfo::TTheme;
		else if (type == "quark")
			packageInfo.Type_ = PackageInfo::TQuark;
		else
			packageInfo.Type_ = PackageInfo::TData;

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
				MakeProperURL (imageNode.attribute ("url"), baseUrl)
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
				MakeProperURL (imageNode.attribute ("url"), baseUrl)
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

		QDomElement verNode = package.firstChildElement ("versions")
				.firstChildElement ("version");
		while (!verNode.isNull ())
		{
			if (verNode.hasAttribute ("size"))
			{
				bool ok = false;
				qint64 size = verNode.attribute ("size").toLong (&ok);
				if (ok)
					packageInfo.PackageSizes_ [verNode.text ()] = size;
			}

			packageInfo.VersionArchivers_ [verNode.text ()] =
					verNode.attribute ("archiver", "gz");

			verNode = verNode.nextSiblingElement ("version");
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
