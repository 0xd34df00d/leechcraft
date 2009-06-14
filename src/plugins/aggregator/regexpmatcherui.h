#ifndef PLUGINS_AGGREGATOR_REGEXPMATCHERUI_H
#define PLUGINS_AGGREGATOR_REGEXPMATCHERUI_H
#include <QDialog>
#include "ui_regexpmatcherui.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class RegexpMatcherUi : public QDialog
			{
				Q_OBJECT

				Ui::RegexpMatcherUi Ui_;
			public:
				RegexpMatcherUi (QWidget* = 0);
				virtual ~RegexpMatcherUi ();
			private slots:
				void on_AddRegexpButton__released ();
				void on_ModifyRegexpButton__released ();
				void on_RemoveRegexpButton__released ();
			};
		};
	};
};

#endif

