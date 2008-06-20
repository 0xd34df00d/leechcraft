#ifndef REGEXPMATCHERUI_H
#define REGEXPMATCHERUI_H
#include <QDialog>
#include "ui_regexpmatcherui.h"

class RegexpMatcherUi : public QDialog
{
	Ui::RegexpMatcherUi Ui_;

	RegexpMatcherUi ();
public:
	virtual ~RegexpMatcherUi ();
	static RegexpMatcherUi& Instance ();
	void Release ();
};

#endif

