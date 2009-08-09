#ifndef PLUGINS_POSHUKU_RELOADINTERVALSELECTOR_H
#define PLUGINS_POSHUKU_RELOADINTERVALSELECTOR_H
#include <QDialog>
#include "ui_reloadintervalselector.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class ReloadIntervalSelector : public QDialog
			{
				Q_OBJECT

				Ui::ReloadIntervalSelector Ui_;
			public:
				ReloadIntervalSelector (QWidget* = 0);

				QTime GetInterval () const;
			};
		};
	};
};

#endif

