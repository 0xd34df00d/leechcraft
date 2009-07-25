#ifndef PLUGINS_POSHUKU_SEARCHTEXT_H
#define PLUGINS_POSHUKU_SEARCHTEXT_H
#include <QDialog>
#include "ui_searchtext.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class SearchText : public QDialog
			{
				Q_OBJECT

				Ui::SearchText Ui_;
				QString Text_;
			public:
				SearchText (const QString&, QWidget* = 0);
			private slots:
				void doSearch ();
				void on_MarkAll__released ();
				void on_UnmarkAll__released ();
			};
		};
	};
};

#endif

