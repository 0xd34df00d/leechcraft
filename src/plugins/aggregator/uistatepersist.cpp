#include "uistatepersist.h"
#include "xmlsettingsmanager.h"
#include <QTreeView>
#include <QString>
#include <QSettings>
#include <QDebug>

namespace LeechCraft
{
namespace Plugins
{
namespace Aggregator
{

void SaveColumnWidth(const QTreeView* tree, const QString& keyName)
{
//QSettings * s=XmlSettingsManager::Instance()->BeginSettings();
qDebug() << Q_FUNC_INFO << keyName;
}

void LoadColumnWidth(const QTreeView* tree, const QString& keyName)
{
//QSettings * s=XmlSettingsManager::Instance()->BeginSettings();
qDebug() << Q_FUNC_INFO << keyName;

}
}
}
}
