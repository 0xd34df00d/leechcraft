#include <QPainter>
#include <QtDebug>
#include <cmath>
#include "particle.h"

Particle::Particle (double x, double y, double speed)
: Angle_ (0)
, Speed_ (speed)
, AngleSpeed_ (0)
, X_ (x)
, Y_ (y)
, Weight_ (0)
{
}

Particle::~Particle ()
{
}

void Particle::RotateBy (double angle)
{
    Angle_ += angle;
}

void Particle::SpeedUp (double speed)
{
    Speed_ += speed;
}

void Particle::Step ()
{
    Angle_ += AngleSpeed_;
    X_ += Speed_ * std::sin (Angle_ * pi / 180);
    Y_ += Speed_ * std::cos (Angle_ * pi / 180);
    if (Weight_ < 100)
        Weight_ += 0.1;
}

double Particle::GetX () const
{
    return X_;
}

double Particle::GetY () const
{
    return Y_;
}

Ship::Ship (double x, double y, double speed)
: Particle (x, y, speed)
{
    Weight_ = 100;
}

Ship::~Ship ()
{
}

QImage Ship::Draw () const
{
    QImage image (":/resources/images/ship.png");
    QPainter paint;
    paint.begin (&image);
    paint.setPen (Qt::white);
    paint.drawRect (1, 1, image.width () - 1, image.height () - 1);
    paint.end ();
    QMatrix matrix;
    matrix.rotate (Angle_ * pi / 180);
    return image.transformed (matrix, Qt::SmoothTransformation);
}

