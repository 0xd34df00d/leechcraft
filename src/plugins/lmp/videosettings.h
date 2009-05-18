#ifndef PLUGINS_LMP_VIDEOSETTINGS_H
#define PLUGINS_LMP_VIDEOSETTINGS_H
#include <QDialog>
#include "ui_videosettings.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class VideoSettings : public QDialog
			{
				Q_OBJECT

				Ui::VideoSettings Ui_;
			public:
				VideoSettings (qreal, qreal, qreal, qreal, QWidget* = 0);
				virtual ~VideoSettings ();
				qreal Brightness () const;
				qreal Contrast () const;
				qreal Hue () const;
				qreal Saturation () const;
			};
		};
	};
};

#endif

