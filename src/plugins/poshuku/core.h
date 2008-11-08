#ifndef CORE_H
#define CORE_H
#include <memory>
#include <vector>
#include <QObject>
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

	Core ();
public:
	static Core& Instance ();
	void Release ();

	bool IsValidURL (const QString&) const;
	BrowserWidget* NewURL (const QString&);
	CustomWebView* MakeWebView ();
	FavoritesModel* GetFavoritesModel () const;
	TagsCompletionModel* GetFavoritesTagsCompletionModel () const;
private slots:
	void handleTitleChanged (const QString&);
	void handleIconChanged (const QIcon&);
	void handleNeedToClose ();
	void handleAddToFavorites (const QString&, const QString&);
	void favoriteTagsUpdated (const QStringList&);
signals:
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
	void changeTabIcon (QWidget*, const QIcon&);
};

#endif

