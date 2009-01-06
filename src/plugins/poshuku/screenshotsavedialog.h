#ifndef SCREENSHOTSAVEDIALOG_H
#define SCREENSHOTSAVEDIALOG_H
#include <QDialog>
#include <QPixmap>
#include "ui_screenshotsavedialog.h"

class ScreenShotSaveDialog : public QDialog
{
	Q_OBJECT
	
	Ui::ScreenShotSaveDialog Ui_;
	QPixmap Source_;
	mutable QPixmap Rendered_;
	mutable QLabel *PixmapHolder_;

	bool RenderScheduled_;
public:
	ScreenShotSaveDialog (const QPixmap&, QWidget* = 0);
	QByteArray Save ();
private:
	void ScheduleRender ();
private slots:
	void render ();
	void on_QualitySlider__valueChanged ();
	void on_FormatCombobox__currentIndexChanged ();
};

#endif

