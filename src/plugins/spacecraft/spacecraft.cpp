#include <QString>
#include <QStringList>
#include <QtPlugin>
#include <QSettings>
#include <QKeyEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "spacecraft.h"
#include "gamewidget.h"

void SpaceCraft::Init (Proxy *proxy)
{
    setWindowTitle ("SpaceCraft");
    IsShown_ = false;
    Proxy_ = proxy;

    QSettings settings (Proxy_->GetOrganizationName (), Proxy_->GetApplicationName ());
    settings.beginGroup (GetName ());
    settings.beginGroup ("geometry");
    move (settings.value ("pos", QPoint (10, 10)).toPoint ());
    settings.endGroup ();
    settings.endGroup ();

    resize (800, 600);
    setMinimumSize (800, 600);
    setMaximumSize (800, 600);
    GameWidget *widget = new GameWidget ();
    setCentralWidget (widget);
    widget->DoDelayedInit ();
    connect (this, SIGNAL (rotateLeft ()), widget, SLOT (rotateLeft ()));
    connect (this, SIGNAL (rotateRight ()), widget, SLOT (rotateRight ()));
    connect (this, SIGNAL (speedUp ()), widget, SLOT (speedUp ()));
}

QString SpaceCraft::GetName () const
{
    return "SpaceCraft";
}

QString SpaceCraft::GetInfo () const
{
    return "A damn simple game";
}

QString SpaceCraft::GetStatusbarMessage () const
{
    return "Status?! What status?!";
}

InfoInterface& SpaceCraft::SetID (InfoInterface::ID_t id)
{
    ID_ = id;
    return *this;
}

InfoInterface::ID_t SpaceCraft::GetID () const
{
    return ID_;
}

QStringList SpaceCraft::Provides () const
{
    return QStringList ();
}

QStringList SpaceCraft::Needs () const
{
    return QStringList ();
}

void SpaceCraft::Release ()
{
    QSettings settings (Proxy_->GetOrganizationName (), Proxy_->GetApplicationName ());
    settings.beginGroup (GetName ());
    settings.beginGroup ("geometry");
    settings.setValue ("pos", pos ());
    settings.endGroup ();
    settings.endGroup ();
}

QIcon SpaceCraft::GetIcon () const
{
    return QIcon ();
}

void SpaceCraft::SetParent (QWidget *parent)
{
    setParent (parent);
}

void SpaceCraft::ShowWindow ()
{
    IsShown_ ? hide () : show ();
    IsShown_ = 1 - IsShown_;
}

void SpaceCraft::ShowBalloonTip ()
{
}

void SpaceCraft::keyPressEvent (QKeyEvent *e)
{
    switch (e->key ())
    {
        case Qt::Key_W:
            emit speedUp ();
            e->accept ();
            return;
        case Qt::Key_A:
            emit rotateLeft ();
            e->accept ();
            return;
        case Qt::Key_D:
            emit rotateRight ();
            e->accept ();
            return;
    }

    e->ignore ();
}

Q_EXPORT_PLUGIN2 (leechcraft_spacecraft, SpaceCraft);

