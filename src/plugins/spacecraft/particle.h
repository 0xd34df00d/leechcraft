#ifndef PARTICLE_H
#define PARTICLE_H
#include <QImage>

class Particle
{
protected:
	double Angle_, Speed_, AngleSpeed_;
	double X_, Y_;
	double Weight_;
	const static double pi = 3.141592653589793238;
public:
	Particle (double, double, double);
	virtual ~Particle ();
	virtual void RotateBy (double);
	virtual void SpeedUp (double);
	virtual void Step ();
	virtual double GetX () const;
	virtual double GetY () const;
	virtual QImage Draw () const = 0;
};

class Ship : public Particle
{
public:
	Ship (double, double, double);
	virtual ~Ship ();
	virtual QImage Draw () const;
};

#endif

