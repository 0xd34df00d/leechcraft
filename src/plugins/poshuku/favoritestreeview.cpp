#include "favoritestreeview.h"
#include <QKeyEvent>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			FavoritesTreeView::FavoritesTreeView (QWidget *parent)
			: QTreeView (parent)
			{
			}
			
			FavoritesTreeView::~FavoritesTreeView ()
			{
			}
			
			void FavoritesTreeView::keyPressEvent (QKeyEvent *e)
			{
				if (e->key () == Qt::Key_Delete &&
						selectionModel ()->currentIndex ().isValid ())
					emit deleteSelected (selectionModel ()->currentIndex ());
			
				QTreeView::keyPressEvent (e);
			}
		};
	};
};

