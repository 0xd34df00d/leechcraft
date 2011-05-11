#include <QTreeView>
#include <QString>
#include <QSettings>
#include <QDebug>
#include <QApplication>
#include "uistatepersist.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			void SaveColumnWidth(const QTreeView* tree, const QString& keyName)
			{
			      qDebug() << Q_FUNC_INFO << keyName;
			      // get & convert to relative width
			      double w= tree->width();
			      QList<QVariant> sizes;
			      for (int i=0;i<tree->model()->columnCount();i++)
				  sizes += tree->columnWidth(i)/w;
			      // save (relative) column width
			      QSettings settings(QApplication::organizationName(),QApplication::applicationName()+" Aggregator");
			      settings.beginGroup("tabs-width");
			      settings.setValue(keyName,sizes);
			      settings.endGroup();
			}

			void LoadColumnWidth(QTreeView* tree, const QString& keyName)
			{
			      qDebug() << Q_FUNC_INFO << keyName;
			      // load (relative) column width
			      QSettings settings(QApplication::organizationName(),QApplication::applicationName()+" Aggregator");
			      settings.beginGroup("tabs-width");
			      QList<QVariant> sizes=settings.value(keyName).toList();
			      settings.endGroup();
			      // some checks 
			      if (sizes.size() != tree->model()->columnCount())
			      {
				    qWarning()<< Q_FUNC_INFO <<
					"Column count of tree ("<<tree->model()->columnCount()<<
					") != column count in settings ("<<sizes.size()<<")";
				    return;
			      }
			      // convert to pixels & set
			      double w= tree->width();
			      for (int i=0;i<sizes.size();i++)
			      {
				    if(!sizes.at(i).canConvert(QVariant::Double))
				    {
					  qWarning()<< Q_FUNC_INFO <<
					      "Can`t convert QVariant to double, (sizes["<<i<<"]="<<sizes.at(i) <<")";
					  return;
				    }
				  tree->setColumnWidth(i,static_cast<int>(sizes.at(i).toDouble()*w));
			      }
			}
		}
	}
}
