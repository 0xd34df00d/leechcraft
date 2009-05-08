#ifndef PLUGINS_NETWORKMONITOR_H
#define PLUGINS_NETWORKMONITOR_H
#include <QDialog>
#include <interfaces/iinfo.h>
#include <interfaces/itoolbarembedder.h>
#include <interfaces/iwantnetworkaccessmanager.h>
#include "ui_networkmonitor.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			class RequestModel;

			class Plugin : public QDialog
						 , public IInfo
						 , public IToolBarEmbedder
						 , public IWantNetworkAccessManager
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IToolBarEmbedder IWantNetworkAccessManager)

				Ui::NetworkMonitor Ui_;
				RequestModel *Model_;
				QNetworkAccessManager *NetworkAccessManager_;
				QList<QAction*> Actions_;
			public:
				void Init ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				QList<QAction*> GetActions () const;

				void SetNetworkAccessManager (QNetworkAccessManager*);
			};
		};
	};
};

#endif

