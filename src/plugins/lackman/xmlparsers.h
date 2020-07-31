/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_XMLPARSERS_H
#define PLUGINS_LACKMAN_XMLPARSERS_H
#include "repoinfo.h"

class QUrl;
class QString;

namespace LC
{
namespace LackMan
{
	RepoInfo ParseRepoInfo (const QUrl& url, const QString& data);
	PackageShortInfoList ParseComponent (const QByteArray& data);
	PackageInfo ParsePackage (const QByteArray& data,
			const QUrl& baseUrl,
			const QString& packageName,
			const QStringList& packageVersions);
}
}

#endif
