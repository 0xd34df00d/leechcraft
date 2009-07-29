#ifndef PLUGINS_LCFTP_TABWIDGET_H
#define PLUGINS_LCFTP_TABWIDGET_H
#include <QWidget>
#include <interfaces/imultitabs.h>
#include "ui_tabwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class TabWidget : public QWidget
							, public IMultiTabsWidget
			{
				Q_OBJECT
				Q_INTERFACES (IMultiTabsWidget)

				Ui::TabWidget Ui_;
			public:
				TabWidget (const QUrl& url, const QString& str, QWidget* = 0);

				void Remove ();
				QToolBar* GetToolBar () const;
			private:
				void Setup (Pane*);
				Pane* Other (Pane*);
			private slots:
				void handleDownloadRequested (const QUrl&);
				void handleUploadRequested (const QString&);
			};

			typedef TabWidget *TabWidget_ptr;
		};
	};
};

#endif

