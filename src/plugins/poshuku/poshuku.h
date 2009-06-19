#ifndef PLUGINS_POSHUKU_POSHUKU_H
#define PLUGINS_POSHUKU_POSHUKU_H
#include <memory>
#include <QAction>
#include <QTranslator>
#include <QWidget>
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <interfaces/imultitabs.h>
#include <interfaces/iwebbrowser.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/tagscompleter.h>
#include "filtermodel.h"
#include "historyfiltermodel.h"
#include "ui_poshuku.h"

class QWebView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class Poshuku : public QWidget
						  , public IInfo
						  , public IEmbedTab
						  , public IMultiTabs
						  , public IPluginReady
						  , public IHaveSettings
						  , public IEntityHandler
						  , public IHaveShortcuts
						  , public IWebBrowser
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEmbedTab IMultiTabs IHaveSettings IEntityHandler IPluginReady IWebBrowser IHaveShortcuts)

				Ui::Poshuku Ui_;

				std::auto_ptr<QTranslator> Translator_;
				std::auto_ptr<LeechCraft::Util::TagsCompleter> FavoritesFilterLineCompleter_;
				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
				std::auto_ptr<FilterModel> FavoritesFilterModel_;
				std::auto_ptr<HistoryFilterModel> HistoryFilterModel_;
			public:
				void Init (ICoreProxy_ptr);
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				QIcon GetIcon () const;

				QWidget* GetTabContents ();
				QToolBar* GetToolBar () const;

				QByteArray GetExpectedPluginClass () const;
				void AddPlugin (QObject*);

				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);

				void Open (const QString&);
				IWebWidget* GetWidget () const;

				void SetShortcut (int, const QKeySequence&);
				QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;
			private:
				void SetupActions ();
				void SetupView ();
				void RegisterSettings ();
				void SetupFavoritesFilter ();
				void SetupHistoryFilter ();
			public slots:
				QWebView* createWindow ();
			private slots:
				void on_HistoryView__activated (const QModelIndex&);
				void on_FavoritesView__activated (const QModelIndex&);
				void on_OpenInTabs__released ();
				void translateRemoveFavoritesItem (const QModelIndex&);
				void viewerSettingsChanged ();
				void cacheSettingsChanged ();
				void updateFavoritesFilter ();
				void updateHistoryFilter ();
				void handleError (const QString&);
				void handleNewTab ();
				void handleSettingsClicked (const QString&);
			signals:
				void bringToFront ();
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void raiseTab (QWidget*);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void couldHandle (const LeechCraft::DownloadEntity&, bool*);
			};
		};
	};
};

#endif

