#ifndef POSHUKU_H
#define POSHUKU_H
#include <memory>
#include <QAction>
#include <QTranslator>
#include <QWidget>
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <interfaces/imultitabs.h>
#include <interfaces/iwebbrowser.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iwantnetworkaccessmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/tagscompleter.h>
#include "filtermodel.h"
#include "ui_poshuku.h"

class QWebView;

class Poshuku : public QWidget
			  , public IInfo
			  , public IEmbedTab
			  , public IMultiTabs
			  , public IHaveSettings
			  , public IWantNetworkAccessManager
			  , public IWebBrowser
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab IMultiTabs IHaveSettings IWantNetworkAccessManager IWebBrowser)

	Ui::Poshuku Ui_;

	std::auto_ptr<QTranslator> Translator_;
	std::auto_ptr<LeechCraft::Util::TagsCompleter> FavoritesFilterLineCompleter_;
	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<FilterModel> FavoritesFilterModel_;
	std::auto_ptr<FilterModel> HistoryFilterModel_;
public:
	void Init ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	QIcon GetIcon () const;

	QWidget* GetTabContents ();

	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

	void SetNetworkAccessManager (QNetworkAccessManager*);

	void Open (const QString&);
	IWebWidget* GetWidget () const;
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
};

#endif

