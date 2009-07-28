#ifndef PLUGINS_LCFTP_TABWIDGET_H
#define PLUGINS_LCFTP_TABWIDGET_H
#include <QWidget>
#include "ui_tabwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class TabWidget : public QWidget
			{
				Q_OBJECT

				Ui::TabWidget Ui_;
			public:
				TabWidget (QWidget* = 0);
			};
		};
	};
};

#endif

