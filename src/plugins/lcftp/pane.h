#ifndef PLUGINS_LCFTP_PANE_H
#define PLUGINS_LCFTP_PANE_H
#include <QWidget>
#include "ui_pane.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class Pane : public QWidget
			{
				Q_OBJECT

				Ui::Pane Ui_;
			public:
				Pane (QWidget* = 0);
			};
		};
	};
};

#endif

