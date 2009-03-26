#ifndef CORE_H
#define CORE_H
#include <memory>
#include <vector>
#include <QObject>
#include <QUrl>
#include <QTimer>
#include <plugininterface/tagscompletionmodel.h>
#include <interfaces/structures.h>
#include "favoritesmodel.h"
#include "historymodel.h"
#include "storagebackend.h"
#include "urlcompletionmodel.h"
#include "pluginmanager.h"

class QString;
class QWidget;
class QIcon;
class CustomWebView;
class QWebView;
class BrowserWidget;
class QAbstractItemModel;
class QNetworkReply;
class QNetworkAccessManager;

class Core : public QObject
{
	Q_OBJECT

	typedef std::vector<BrowserWidget*> widgets_t;
	widgets_t Widgets_;

	std::auto_ptr<FavoritesModel> FavoritesModel_;
	std::auto_ptr<HistoryModel> HistoryModel_;
	std::auto_ptr<LeechCraft::Util::TagsCompletionModel> FavoriteTagsCompletionModel_;
	std::auto_ptr<StorageBackend> StorageBackend_;
	std::auto_ptr<URLCompletionModel> URLCompletionModel_;
	std::auto_ptr<PluginManager> PluginManager_;
	QNetworkAccessManager *NetworkAccessManager_;

	QMap<QString, QObject*> Providers_;

	bool SaveSessionScheduled_;
	QStringList RestoredURLs_;

	QMap<QString, QString> SavedSession_;
	QList<QAction*> Unclosers_;

	Core ();
public:
	static Core& Instance ();
	void Release ();
	void SetProvider (QObject*, const QString&);
	QByteArray GetExpectedPluginClass () const;
	void AddPlugin (QObject*);

	QUrl MakeURL (QString) const;
	BrowserWidget* NewURL (const QString&, bool = false);
	CustomWebView* MakeWebView (bool = false);
	void Unregister (BrowserWidget*);

	FavoritesModel* GetFavoritesModel () const;
	HistoryModel* GetHistoryModel () const;
	URLCompletionModel* GetURLCompletionModel () const;
	LeechCraft::Util::TagsCompletionModel* GetFavoritesTagsCompletionModel () const;
	QNetworkAccessManager* GetNetworkAccessManager () const;
	void SetNetworkAccessManager (QNetworkAccessManager*);
	StorageBackend* GetStorageBackend () const;
	PluginManager* GetPluginManager () const;
private:
	void RestoreSession (bool);
	void ScheduleSaveSession ();
	void HandleHistory (QWebView*);
public slots:
	void importXbel ();
	void exportXbel ();
private slots:
	void handleUnclose ();
	void handleTitleChanged (const QString&);
	void handleURLChanged (const QString&);
	void handleIconChanged (const QIcon&);
	void handleNeedToClose ();
	void handleAddToFavorites (const QString&, const QString&);
	void handleStatusBarChanged (const QString&);
	void handleTooltipChanged (QWidget*);
	void favoriteTagsUpdated (const QStringList&);
	void saveSession ();
	void restorePages ();
	void postConstruct ();
signals:
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
	void changeTabIcon (QWidget*, const QIcon&);
	void changeTooltip (QWidget*, QWidget*);
	void raiseTab (QWidget*);
	void error (const QString&) const;
	void statusBarChanged (QWidget*, const QString&);
	void gotEntity (const LeechCraft::DownloadEntity&);
	void couldHandle (const LeechCraft::DownloadEntity&, bool*);
	void newUnclose (QAction*);
};

#endif

