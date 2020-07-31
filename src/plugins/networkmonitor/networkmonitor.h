/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETWORKMONITOR_NETWORKMONITOR_H
#define PLUGINS_NETWORKMONITOR_NETWORKMONITOR_H
#include <QDialog>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include "ui_networkmonitor.h"

class QSortFilterProxyModel;
class QNetworkAccessManager;

namespace LC
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			class RequestModel;

			class Plugin : public QDialog
						 , public IInfo
						 , public IActionsExporter
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IActionsExporter)

				LC_PLUGIN_METADATA ("org.LeechCraft.NetworkMonitor")

				Ui::NetworkMonitor Ui_;
				RequestModel *Model_;
				QSortFilterProxyModel *ProxyModel_;
				QNetworkAccessManager *NetworkAccessManager_;
				QList<QAction*> Actions_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;

				QList<QAction*> GetActions (ActionsEmbedPlace) const;
			public slots:
				void handleCurrentChanged (const QModelIndex&);
				void filterUpdated ();
			signals:
				void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
			};
		};
	};
};

#endif

