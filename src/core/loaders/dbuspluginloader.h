/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include "ipluginloader.h"

class QProcess;
class QDBusInterface;

namespace LC
{
namespace Loaders
{
	class InfoProxy;

	class DBusPluginLoader : public QObject
						   , public IPluginLoader
	{
		Q_OBJECT

		const QString Filename_;
		bool IsLoaded_;

		std::shared_ptr<QProcess> Proc_;
		std::shared_ptr<QDBusInterface> CtrlIface_;
		std::shared_ptr<InfoProxy> Wrapper_;
	public:
		DBusPluginLoader (const QString&);

		quint64 GetAPILevel ();

		bool Load ();
		bool Unload ();

		QObject* Instance ();
		bool IsLoaded () const;
		QString GetFileName () const;
		QString GetErrorString () const;
		QVariantMap GetManifest () const;
	private slots:
		void handleProcFinished ();
	};
}
}
