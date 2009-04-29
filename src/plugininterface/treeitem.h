#ifndef PLUGININTERFACE_TREEITEM_H
#define PLUGININTERFACE_TREEITEM_H
#include <QList>
#include <QVector>
#include <QMap>
#include <QVariant>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem
		{
			QList<TreeItem*> Childs_;
			QMap<int, QVector<QVariant> > Data_;
			TreeItem *Parent_;
		public:
			PLUGININTERFACE_API TreeItem (const QList<QVariant>&, TreeItem *parent = 0);
			PLUGININTERFACE_API ~TreeItem ();

			PLUGININTERFACE_API void AppendChild (TreeItem*);
			PLUGININTERFACE_API void PrependChild (TreeItem*);
			PLUGININTERFACE_API int ChildPosition (TreeItem*);
			PLUGININTERFACE_API void RemoveChild (int);
			PLUGININTERFACE_API TreeItem* Child (int);
			PLUGININTERFACE_API int ChildCount () const;
			PLUGININTERFACE_API int ColumnCount (int = Qt::DisplayRole) const;
			PLUGININTERFACE_API QVariant Data (int, int = Qt::DisplayRole) const;
			PLUGININTERFACE_API void ModifyData (int, const QVariant&, int = Qt::DisplayRole);
			PLUGININTERFACE_API TreeItem* Parent ();
			PLUGININTERFACE_API int Row () const;
		};
	};
};

#endif

