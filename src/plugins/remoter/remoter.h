#ifndef REMOTER_H
#define REMOTER_H
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include "ui_mainwindow.h"

class Remoter : public QMainWindow
              , public IInfo
              , public IWindow
{
    Q_OBJECT
    Q_INTERFACES (IInfo IWindow);

    Ui::MainWindow Ui_;
    unsigned long int ID_;
    bool IsShown_;

    QObjectList Objects_;
public:
    void Init ();
    void Release ();
    QString GetName () const;
    QString GetInfo () const;
    QString GetStatusbarMessage () const;
    IInfo& SetID (long unsigned int);
    unsigned long int GetID () const;    
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    void PushMainWindowExternals (const MainWindowExternals&);
    QIcon GetIcon () const;
    void SetParent (QWidget*);
    void ShowWindow ();
    void ShowBalloonTip ();
protected:
    virtual void closeEvent (QCloseEvent*);
};

#endif

