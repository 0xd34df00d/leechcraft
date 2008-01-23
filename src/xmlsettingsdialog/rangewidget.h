#ifndef RANGEWIDGET_H
#define RANGEWIDGET_h
#include <QWidget>

class QSpinBox;

class RangeWidget : public QWidget
{
	Q_OBJECT

	QSpinBox *Lower_, *Higher_;
public:
	RangeWidget (QWidget *parent = 0);
	void SetMinimum (int);
	void SetMaximum (int);
	void SetLower (int);
	void SetHigher (int);
	void SetRange (const QVariant&);
	QVariant GetRange () const;
private slots:
	void lowerChanged (int);
	void upperChanged (int);
signals:
	void changed ();
};

#endif

