#ifndef SPACECRAFT_H
#define SPACECRAFT_H
#include <interfaces/interfaces.h>
#include <QMainWindow>

class SpaceCraft : public QMainWindow
     , public InfoInterface
     , public WindowInterface
{
 Q_OBJECT
 Q_INTERFACES (InfoInterface WindowInterface);

 ID_t ID_;
 bool IsShown_;

 Proxy *Proxy_;
public:
 virtual void Init (Proxy*);
 virtual QString GetName () const;
 virtual QString GetInfo () const;
 virtual QString GetStatusbarMessage () const;
 virtual InfoInterface& SetID (ID_t);
 virtual ID_t GetID () const;
 virtual QStringList Provides () const;
 virtual QStringList Needs () const;
 virtual void Release ();
 virtual QIcon GetIcon () const;
 virtual void SetParent (QWidget*);
 virtual void ShowWindow ();
 virtual void ShowBalloonTip ();
protected slots:
 virtual void keyPressEvent (QKeyEvent*);
signals:
 void rotateLeft ();
 void rotateRight ();
 void speedUp ();
 void fire ();
};

#endif

