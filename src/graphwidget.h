#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H
#include <QWidget>
#include <QList>

namespace LeechCraft
{
	namespace Util
	{
		class GraphWidget : public QWidget
		{
			Q_OBJECT

			QList<quint64> DownSpeeds_;
			QList<quint64> UpSpeeds_;
			QColor DownColor_;
			QColor UpColor_;
		public:
			GraphWidget (const QColor&, const QColor&,
					QWidget *parent = 0);

			void PushSpeed (quint64, quint64);
		protected:
			virtual void paintEvent (QPaintEvent*);
		private:
			virtual void PaintSingle (quint64, const QList<quint64>&,
					QPainter*);
		};
	};
};

#endif

