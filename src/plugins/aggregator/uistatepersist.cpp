#include "uistatepersist.h"
#include <QTreeView>
#include <QString>
#include <QSettings>
#include <QDebug>
#include <QApplication>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			void SaveColumnWidth (const QTreeView *tree, const QString &keyName)
			{
				// get width
				QList<QVariant> sizes;
				for (int i = 0, count = tree->model ()->columnCount (); i < count; ++i)
					sizes += tree->columnWidth (i);
				// save column width
				QSettings settings (QApplication::organizationName (), QApplication::applicationName () + " Aggregator");
				settings.beginGroup ("tabs-width");
				settings.setValue (keyName, sizes);
				settings.endGroup ();
			}

			void LoadColumnWidth (QTreeView *tree, const QString &keyName)
			{
				// load column width
				QSettings settings (QApplication::organizationName (), QApplication::applicationName () + " Aggregator");
				settings.beginGroup ("tabs-width");
				QList<QVariant> sizes = settings.value (keyName).toList ();
				settings.endGroup ();
				// some checks 
				if (sizes.size () != tree->model ()->columnCount ())
				{
					qWarning () << Q_FUNC_INFO <<
						"Column count of tree (" << tree->model ()->columnCount () <<
						") != column count in settings (" << sizes.size () << ")";
					return;
				}
				// set width
				const int minColumnSize=4;
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
					if (s < minColumnSize)
					{
						qWarning() << Q_FUNC_INFO << "Size of column #" << i <<
							"(" << s << ") is too small (min." << minColumnSize << ")";
						continue;
					}
					tree->setColumnWidth (i, s);
				}
			}
		}
	}
}

