#include "progresslineedit.h"

ProgressLineEdit::ProgressLineEdit (QWidget *parent)
: QLineEdit (parent)
{
	setValue (0);
}

ProgressLineEdit::~ProgressLineEdit ()
{
}

void ProgressLineEdit::setValue (int value)
{
	double sv = static_cast<double> (value * 2) / 100;
	sv -= 1.001;
	QString ss = QString ("QLineEdit { "
			"background-color: qlineargradient(spread:pad, "
			"x1:0, y1:1, x2:1, y2:1, "
			"stop:%1 rgba(0, 120, 0, 180), "
			"stop:1 rgba(255, 255, 255, 255)) }")
		.arg (sv);

	setStyleSheet (ss);
}

