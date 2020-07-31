/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_COREPROXYWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_COREPROXYWRAPPER_H
#include <interfaces/core/icoreproxy.h>
#include <QMap>
#include <QIcon>
#include <QStringList>
#include <QModelIndex>

class QTreeView;
class QTabWidget;
class QNetworkAccessManager;
class ICoreTabWidget;

Q_DECLARE_METATYPE (QNetworkAccessManager*)

namespace LC
{
namespace Qrosp
{
	class CoreProxyWrapper : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
	public:
		CoreProxyWrapper (ICoreProxy_ptr);
	public slots:
		QNetworkAccessManager* GetNetworkAccessManager () const;
		QObject* GetShortcutProxy () const;
		QModelIndex MapToSource (const QModelIndex&) const;
		//LC::Util::BaseSettingsManager* GetSettingsManager () const;
		QIcon GetIcon (const QString& on, const QString& off = QString ()) const;
		QMainWindow* GetMainWindow () const;
		ICoreTabWidget* GetTabWidget () const;
		QObject* GetTagsManager () const;
		QStringList GetSearchCategories () const;
		QObject* GetPluginsManager () const;
	};
}
}

#endif
