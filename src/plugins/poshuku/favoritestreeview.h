#ifndef FAVORITESTREEVIEW_H
#define FAVORITESTREEVIEW_H
#include <QTreeView>

class FavoritesTreeView : public QTreeView
{
	Q_OBJECT
public:
	FavoritesTreeView (QWidget* = 0);
	virtual ~FavoritesTreeView ();
protected:
	virtual void keyPressEvent (QKeyEvent*);
signals:
	void deleteSelected (const QModelIndex&);
};

#endif

