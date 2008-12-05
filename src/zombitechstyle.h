#ifndef ZOMBITECHSTYLE_H
#define ZOMBITECHSTYLE_H
#include <QPlastiqueStyle>

class ZombiTechStyle : public QPlastiqueStyle
{
	Q_OBJECT

	QColor White_;
	QColor MidOrange_;
	QColor TextOrange_;
	QColor DarkOrange_;
	QColor Background_;
	QColor LightOrange_;
	QColor BorderOrange_;
	QColor DarkTextOrange_;
public:
	ZombiTechStyle ();
	virtual ~ZombiTechStyle ();

	virtual void polish (QPalette&);
	virtual void polish (QWidget*);
	virtual void unpolish (QWidget*);
	virtual int pixelMetric (PixelMetric, const QStyleOption*,
			const QWidget*) const;
	virtual int styleHint (StyleHint, const QStyleOption*,
			const QWidget*, QStyleHintReturn*) const;
	virtual void drawPrimitive (PrimitiveElement,
			const QStyleOption*, QPainter*, const QWidget*) const;
};

#endif

