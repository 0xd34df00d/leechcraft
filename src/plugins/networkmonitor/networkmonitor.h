#ifndef PLUGINS_NETWORKMONITOR_NETWORKMONITOR_H
#define PLUGINS_NETWORKMONITOR_NETWORKMONITOR_H
#include <QDialog>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/itoolbarembedder.h>
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
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IToolBarEmbedder)

				Ui::NetworkMonitor Ui_;
				RequestModel *Model_;
				QNetworkAccessManager *NetworkAccessManager_;
				QList<QAction*> Actions_;
				std::auto_ptr<QTranslator> Translator_;
			public:
				void Init (ICoreProxy_ptr);
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				QList<QAction*> GetActions () const;
			};
		};
	};
};

#endif

