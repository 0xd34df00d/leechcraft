#ifndef UISTATEPERSIST_H
#define UISTATEPERSIST_H
#include <QTreeView>
#include <QString>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			/* Save column width of tree to aggregator`s section of settings */
			void SaveColumnWidth (const QTreeView * tree, const QString &keyName);
			/* Try to load column width of tree from aggregator`s section of settings */
			void LoadColumnWidth (const QTreeView * tree, const QString &keyName);
		}
	}
}

#endif

