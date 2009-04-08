#ifndef SETTINGS_H
#define SETTINGS_H
#include <QWidget>
#include "ui_settings.h"

class Settings : public QWidget
{
	Q_OBJECT

	Ui::Settings Ui_;
public:
	Settings (QAbstractItemModel*, QWidget* = 0);
};

#endif

