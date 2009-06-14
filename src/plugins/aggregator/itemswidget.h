#ifndef PLUGINS_AGGREGATOR_ITEMSWIDGET_H
#define PLUGINS_AGGREGATOR_ITEMSWIDGET_H
#include <QWidget>
#include "ui_itemswidget.h"
#include "item.h"

class QModelIndex;
class IWebBrowser;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct ItemsWidget_Impl;

			class ItemsWidget : public QWidget
			{
				Q_OBJECT

				ItemsWidget_Impl *Impl_;
			public:
				ItemsWidget (QWidget* = 0);
				virtual ~ItemsWidget ();

				void SetTapeMode (bool);
				void SetHideRead (bool);
				void HideInfoPanel ();
			private:
				void Construct (bool);
				QString GetHex (QPalette::ColorRole,
						QPalette::ColorGroup = QApplication::palette ().currentColorGroup ());
				QString ToHtml (const Item_ptr&);
			private slots:
				void channelChanged (const QModelIndex&);
				void on_ActionMarkItemAsUnread__triggered ();
				void on_CaseSensitiveSearch__stateChanged (int);
				void on_ActionAddToItemBucket__triggered ();
				void on_ItemCommentsSubscribe__released ();
				void on_ItemCategoriesButton__released ();
				void currentItemChanged (const QItemSelection&);
				void makeCurrentItemVisible ();
				void updateItemsFilter ();
			};
		};
	};
};

#endif

