#ifndef CORE_H
#define CORE_H
#include <vector>
#include <QObject>

class QString;
class QWidget;
class QIcon;
class CustomWebView;
class BrowserWidget;
class QAbstractItemModel;
class FavoritesModel;

class Core : public QObject
{
	Q_OBJECT

	std::vector<BrowserWidget*> Widgets_;

	FavoritesModel *FavoritesModel_;

	Core ();
public:
	static Core& Instance ();
	void Release ();

	bool IsValidURL (const QString&) const;
	BrowserWidget* NewURL (const QString&);
	CustomWebView* MakeWebView ();
	QAbstractItemModel* GetFavoritesModel () const;
private slots:
	void handleTitleChanged (const QString&);
	void handleIconChanged (const QIcon&);
	void handleNeedToClose ();
	void handleAddToFavorites (const QString&, const QString&);
signals:
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
	void changeTabIcon (QWidget*, const QIcon&);
};

#endif

