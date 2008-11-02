#ifndef VIDEOSETTINGS_H
#define VIDEOSETTINGS_H
#include <QDialog>
#include "ui_videosettings.h"

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

#endif

