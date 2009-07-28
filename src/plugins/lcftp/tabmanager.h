#ifndef PLUGINS_LCFTP_TABMANAGER_H
#define PLUGINS_LCFTP_TABMANAGER_H
#include <QObject>
#include "tabwidget.h"

class QUrl;
class QString;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class TabManager : public QObject
			{
				Q_OBJECT

				QList<TabWidget_ptr> Widgets_;
			public:
				TabManager (QObject* = 0);
				~TabManager ();

				void AddTab (const QUrl&, const QString&);
				void Remove (TabWidget*);
			signals:
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void statusBarChanged (QWidget*, const QString&);
			};
		};
	};
};

#endif

