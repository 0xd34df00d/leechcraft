#ifndef CORE_H
#define CORE_H
#include <memory>
#include <vector>
#include <QObject>
#include <QUrl>
#include <QTimer>
#include <plugininterface/tagscompletionmodel.h>
#include "favoritesmodel.h"
#include "historymodel.h"
#include "storagebackend.h"
#include "urlcompletionmodel.h"

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

	QUrl MakeURL (QString) const;
	BrowserWidget* NewURL (const QString&, bool = false);
	CustomWebView* MakeWebView (bool = false);
	FavoritesModel* GetFavoritesModel () const;
	HistoryModel* GetHistoryModel () const;
	URLCompletionModel* GetURLCompletionModel () const;
	LeechCraft::Util::TagsCompletionModel* GetFavoritesTagsCompletionModel () const;
	QNetworkAccessManager* GetNetworkAccessManager () const;
	void SetNetworkAccessManager (QNetworkAccessManager*);
	StorageBackend* GetStorageBackend () const;
	void Unregister (BrowserWidget*);
private:
	void RestoreSession (bool);
	void ScheduleSaveSession ();
	void HandleHistory (QWebView*);
private slots:
	void handleUnclose ();
	void handleTitleChanged (const QString&);
	void handleURLChanged (const QString&);
	void handleIconChanged (const QIcon&);
	void handleNeedToClose ();
	void handleAddToFavorites (const QString&, const QString&);
	void handleStatusBarChanged (const QString&);
	void favoriteTagsUpdated (const QStringList&);
	void saveSession ();
	void restorePages ();
	void postConstruct ();
signals:
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
	void changeTabIcon (QWidget*, const QIcon&);
	void raiseTab (QWidget*);
	void error (const QString&) const;
	void statusBarChanged (QWidget*, const QString&);
	void gotEntity (const QByteArray&);
	void newUnclose (QAction*);
};

#endif

