#ifndef ITEMSWIDGET_H
#define ITEMSWIDGET_H
#include <QWidget>
#include "ui_itemswidget.h"
#include "item.h"

struct ItemsWidget_Impl;
class QModelIndex;
class IWebBrowser;

class ItemsWidget : public QWidget
{
	Q_OBJECT

	ItemsWidget_Impl *Impl_;
public:
	ItemsWidget (QWidget* = 0);
	virtual ~ItemsWidget ();

	void SetHideRead (bool);
	void ChannelChanged (const QModelIndex&);
	void HideInfoPanel ();
private:
	void Construct (bool);
	void SetHtml (const Item_ptr&);
private slots:
    void on_ActionMarkItemAsUnread__triggered ();
	void on_CaseSensitiveSearch__stateChanged (int);
	void on_ActionAddToItemBucket__triggered ();
	void on_ItemCommentsSubscribe__released ();
	void on_ItemCategoriesButton__released ();
    void currentItemChanged (const QItemSelection&);
	void makeCurrentItemVisible ();
	void updateItemsFilter ();
};

#endif

