/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMultiMap>
#include <QRegularExpression>
#include <interfaces/poshuku/iproxyobject.h>

class QNetworkAccessManager;

namespace LC
{
namespace Poshuku
{
class IWebView;

namespace FatApe
{
	class UserScript
	{
		QString ScriptPath_;
		QRegularExpression MetadataRX_;
		QMultiMap<QString, QString> Metadata_;
		bool Enabled_;
	public:
		explicit UserScript (const QString& scriptPath);

		bool MatchToPage (const QString& pageUrl) const;
		void Inject (IWebView *frame, IProxyObject *proxy) const;
		QString Name () const;
		QString Description () const;
		QString Namespace () const;
		QString GetResourcePath (const QString& resourceName) const;
		QString Path () const;
		QStringList Include () const;
		QStringList Exclude () const;
		bool IsEnabled () const;
		void SetEnabled (bool value);

		bool Install ();
		void Install (QNetworkAccessManager *networkManager);

		void Delete ();
	private:
		void ParseMetadata ();
		void DownloadResource (const QString& resource, QNetworkAccessManager *networkManager);
		void DownloadRequired (const QString& required, QNetworkAccessManager *networkManager);
    };
}
}
}
