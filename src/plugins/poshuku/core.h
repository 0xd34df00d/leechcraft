#ifndef CORE_H
#define CORE_H
#include <memory>
#include <vector>
#include <QObject>
#include <QNetworkAccessManager>
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

class Core : public QObject
{
	Q_OBJECT

	typedef std::vector<BrowserWidget*> widgets_t;
	widgets_t Widgets_;

	std::auto_ptr<FavoritesModel> FavoritesModel_;
	std::auto_ptr<HistoryModel> HistoryModel_;
	std::auto_ptr<LeechCraft::Util::TagsCompletionModel> FavoriteTagsCompletionModel_;
	std::auto_ptr<QNetworkAccessManager> NetworkAccessManager_;
	std::auto_ptr<QTimer> CookieSaveTimer_;
	std::auto_ptr<StorageBackend> StorageBackend_;
	std::auto_ptr<URLCompletionModel> URLCompletionModel_;

	QMap<QString, QObject*> Providers_;
	QObjectList Downloaders_;

	bool SaveSessionScheduled_;
	QStringList RestoredURLs_;

	QMap<QString, QString> SavedSession_;

	Core ();
public:
	static Core& Instance ();
	void Release ();
	void SetProvider (QObject*, const QString&);

	bool IsValidURL (const QString&) const;
	BrowserWidget* NewURL (const QString&);
	CustomWebView* MakeWebView ();
	FavoritesModel* GetFavoritesModel () const;
	HistoryModel* GetHistoryModel () const;
	URLCompletionModel* GetURLCompletionModel () const;
	LeechCraft::Util::TagsCompletionModel* GetFavoritesTagsCompletionModel () const;
	QNetworkAccessManager* GetNetworkAccessManager () const;
	StorageBackend* GetStorageBackend () const;
	void Unregister (BrowserWidget*);
	void GotLink (const QString&);
private:
	void DoCommonAuth (const QString&, QAuthenticator*);
	void RestoreSession (bool);
	void ScheduleSaveSession ();
	void HandleHistory (QWebView*);
public slots:
	void gotUnsupportedContent (const QByteArray& = QByteArray ());
private slots:
	void saveCookies () const;
	void handleTitleChanged (const QString&);
	void handleURLChanged (const QString&);
	void handleIconChanged (const QIcon&);
	void handleNeedToClose ();
	void handleAddToFavorites (const QString&, const QString&);
	void handleAuthentication (QNetworkReply*, QAuthenticator*);
	void handleProxyAuthentication (const QNetworkProxy&, QAuthenticator*);
	void handleSslErrors (QNetworkReply*, const QList<QSslError>&);
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
};

#endif

