#ifndef TREEITEM_H
#define TREEITEM_H
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
			LEECHCRAFT_API TreeItem (const QList<QVariant>&, TreeItem *parent = 0);
			LEECHCRAFT_API ~TreeItem ();

			LEECHCRAFT_API void AppendChild (TreeItem*);
			LEECHCRAFT_API void PrependChild (TreeItem*);
			LEECHCRAFT_API int ChildPosition (TreeItem*);
			LEECHCRAFT_API void RemoveChild (int);
			LEECHCRAFT_API TreeItem* Child (int);
			LEECHCRAFT_API int ChildCount () const;
			LEECHCRAFT_API int ColumnCount (int = Qt::DisplayRole) const;
			LEECHCRAFT_API QVariant Data (int, int = Qt::DisplayRole) const;
			LEECHCRAFT_API void ModifyData (int, const QVariant&, int = Qt::DisplayRole);
			LEECHCRAFT_API TreeItem* Parent ();
			LEECHCRAFT_API int Row () const;
		};
	};
};

#endif

