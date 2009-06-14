#ifndef PLUGINS_POSHUKU_FAVORITESTREEVIEW_H
#define PLUGINS_POSHUKU_FAVORITESTREEVIEW_H
#include <QTreeView>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
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
		};
	};
};

#endif

