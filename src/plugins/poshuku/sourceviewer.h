#ifndef PLUGINS_POSHUKU_SOURCEVIEWER_H
#define PLUGINS_POSHUKU_SOURCEVIEWER_H
#include <QMainWindow>
#include "ui_sourceviewer.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class SourceViewer : public QMainWindow
			{
				Q_OBJECT

				Ui::SourceViewer Ui_;
			public:
				SourceViewer (QWidget* = 0);
				void SetHtml (const QString&);
			};
		};
	};
};

#endif

