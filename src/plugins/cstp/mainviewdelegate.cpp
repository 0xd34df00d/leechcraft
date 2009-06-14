#include "mainviewdelegate.h"
#include <QApplication>
#include <plugininterface/proxy.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			MainViewDelegate::MainViewDelegate (QWidget *parent)
			: QItemDelegate (parent)
			{
			}
			
			void MainViewDelegate::paint (QPainter *painter,
					const QStyleOptionViewItem& option,
					const QModelIndex& index) const
			{
				if (index.column () != Core::HProgress)
				{
					QItemDelegate::paint (painter, option, index);
					return;
				}
				
				QStyleOptionProgressBar pbo;
				pbo.state = QStyle::State_Enabled;
				pbo.direction = QApplication::layoutDirection ();
				pbo.rect = option.rect;
				pbo.fontMetrics = QApplication::fontMetrics ();
				pbo.minimum = 0;
				pbo.maximum = 100;
				pbo.textAlignment = Qt::AlignCenter;
				pbo.textVisible = true;
			
				bool isr = Core::Instance ().IsRunning (index.row ());
			
				if (isr)
				{
					qint64 done = Core::Instance ().GetDone (index.row ()),
						   total = Core::Instance ().GetTotal (index.row ());
					int progress = total ? done * 100 / total : 0;
					pbo.progress = progress;
					pbo.text = QString ("%1 (%2 of %3)")
						.arg (progress)
						.arg (LeechCraft::Util::Proxy::Instance ()->MakePrettySize (done))
						.arg (LeechCraft::Util::Proxy::Instance ()->MakePrettySize (total));
				}
				else
				{
					pbo.progress = 0;
					pbo.text = QString (tr ("Idle"));
				}
			}
		};
	};
};

