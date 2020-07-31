/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

class QNetworkReply;

namespace LC
{
namespace Poshuku
{
namespace Autosearch
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.Autosearch")

		ICoreProxy_ptr Proxy_;

		QMap<QNetworkReply*, QObject*> Reply2Model_;
		QMap<QObject*, QNetworkReply*> Model2Reply_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
	public slots:
		void hookURLCompletionNewStringRequested (LC::IHookProxy_ptr proxy,
				QObject *model,
				const QString& string,
				int historyItems);
	private slots:
		void handleReply ();
	};
}
}
}
