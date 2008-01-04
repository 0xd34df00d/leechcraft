#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H
#include <QWidget>
#include <QVector>
#include <QPoint>
class QPaintEngine;
class QPainter;
class Ship;
class Particle;

class GameWidget : public QWidget
{
	Q_OBJECT

	QPaintEngine *PaintEngine_;
	QPainter *Painter_;
	QVector<QPoint> Stars_;
	Ship *Ship_;
	QList<Particle*> Particles_;

	bool FirstUpdate_;
public:
	GameWidget (QWidget *parent = 0);
	void DoDelayedInit ();
	~GameWidget ();
public slots:
	void rotateLeft ();
	void rotateRight ();
	void speedUp ();
protected:
	void paintEvent (QPaintEvent*);
};

#endif

