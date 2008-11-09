#ifndef CORE_H
#define CORE_H
#include <memory>
#include <vector>
#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <plugininterface/tagscompletionmodel.h>
#include "favoritesmodel.h"

class QString;
class QWidget;
class QIcon;
class CustomWebView;
class BrowserWidget;
class QAbstractItemModel;

class Core : public QObject
{
	Q_OBJECT

	std::vector<BrowserWidget*> Widgets_;

	std::auto_ptr<FavoritesModel> FavoritesModel_;
	std::auto_ptr<TagsCompletionModel> FavoriteTagsCompletionModel_;
	std::auto_ptr<QNetworkAccessManager> NetworkAccessManager_;
	std::auto_ptr<QTimer> CookieSaveTimer_;

	Core ();
public:
	static Core& Instance ();
	void Release ();

	bool IsValidURL (const QString&) const;
	BrowserWidget* NewURL (const QString&);
	CustomWebView* MakeWebView ();
	FavoritesModel* GetFavoritesModel () const;
	TagsCompletionModel* GetFavoritesTagsCompletionModel () const;
	QNetworkAccessManager* GetNetworkAccessManager () const;
private:
	void DoCommonAuth (const QString&, QAuthenticator*);
private slots:
	void saveCookies () const;
	void handleTitleChanged (const QString&);
	void handleIconChanged (const QIcon&);
	void handleNeedToClose ();
	void handleAddToFavorites (const QString&, const QString&);
	void handleAuthentication (QNetworkReply*, QAuthenticator*);
	void handleProxyAuthentication (const QNetworkProxy&, QAuthenticator*);
	void handleSslErrors (QNetworkReply*, const QList<QSslError>&);
	void favoriteTagsUpdated (const QStringList&);
signals:
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
	void changeTabIcon (QWidget*, const QIcon&);
	void error (const QString&) const;
};

#endif

