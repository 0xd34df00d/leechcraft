#ifndef PLUGINS_SEEKTHRU_SEARCHERSLIST_H
#define PLUGINS_SEEKTHRU_SEARCHERSLIST_H
#include <QWidget>
#include "ui_searcherslist.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class SearchersList : public QWidget
			{
				Q_OBJECT

				Ui::SearchersList Ui_;
				QModelIndex Current_;
			public:
				SearchersList (QWidget* = 0);
			private slots:
				void handleCurrentChanged (const QModelIndex&);
				void on_ButtonAdd__released ();
				void on_ButtonRemove__released ();
				void on_Tags__textEdited (const QString&);
			};
		};
	};
};

#endif

