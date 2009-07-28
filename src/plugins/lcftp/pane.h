#ifndef PLUGINS_LCFTP_PANE_H
#define PLUGINS_LCFTP_PANE_H
#include <boost/shared_ptr.hpp>
#include <QWidget>
#include "ui_pane.h"

class QDirModel;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class Worker;

			class Pane : public QWidget
			{
				Q_OBJECT

				Ui::Pane Ui_;

				boost::shared_ptr<Worker> Worker_;
				QDirModel *DirModel_;
			public:
				Pane (QWidget* = 0);
				virtual ~Pane ();

				void SetURL (const QUrl&);
				void Navigate (const QString&);
			};
		};
	};
};

#endif

