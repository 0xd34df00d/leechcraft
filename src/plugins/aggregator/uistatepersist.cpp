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
			void SaveColumnWidth (const QTreeView* tree, const QString& keyName)
			{
				// get width
				QList<QVariant> sizes;
				for (int i = 0; i < tree->model ()->columnCount (); i++)
				{
					sizes += tree->columnWidth (i);
				}
				qDebug() << Q_FUNC_INFO << keyName << "sizes=" << sizes;
				// save column width
				QSettings settings (QApplication::organizationName (), QApplication::applicationName () + " Aggregator");
				settings.beginGroup ("tabs-width");
				settings.setValue (keyName, sizes);
				settings.endGroup ();
			}

			void LoadColumnWidth (QTreeView* tree, const QString& keyName)
			{
				// load column width
				QSettings settings (QApplication::organizationName (), QApplication::applicationName () + " Aggregator");
				settings.beginGroup ("tabs-width");
				QList<QVariant> sizes = settings.value (keyName).toList ();
				settings.endGroup ();
				qDebug () << Q_FUNC_INFO << keyName << "sizes=" << sizes;
				// some checks 
				if (sizes.size () != tree->model ()->columnCount ())
				{
					qWarning () << Q_FUNC_INFO <<
						"Column count of tree (" << tree->model ()->columnCount () <<
						") != column count in settings (" << sizes.size () << ")";
					return;
				}
				// set width
				const int MIN_COLUMN_SIZE=4;
				for (int i = 0; i < sizes.size (); i++)
				{
					// checks
					if (!sizes.at (i).canConvert (QVariant::Int))
					{
						qWarning() << Q_FUNC_INFO << "Can`t convert QVariant to int, "
							"(sizes[" << i << "]=" << sizes.at (i) << ")";
						return;
					}
					int s = sizes.at (i).toInt ();
					if (s < MIN_COLUMN_SIZE)
					{
						qWarning() << Q_FUNC_INFO << "Size of column #" << i <<
							"(" << s << ") is too small (min." << MIN_COLUMN_SIZE << ")";
						continue;
					}
					tree->setColumnWidth (i, s);
				}
				qDebug() << Q_FUNC_INFO << keyName<< ": loaded successful";
			}
		}
	}
}

