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
void SaveColumnWidth(const QTreeView * tree,const QString &keyName);
void LoadColumnWidth(const QTreeView * tree,const QString &keyName);
}
}
}

#endif
