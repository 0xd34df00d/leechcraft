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
    bool IsShown_;

    QObjectList Objects_;
public:
    void Init ();
    void Release ();
    QString GetName () const;
    QString GetInfo () const;
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    QIcon GetIcon () const;
    void SetParent (QWidget*);
    void ShowWindow ();
protected:
    virtual void closeEvent (QCloseEvent*);
public slots:
    void handleHidePlugins ();
};

#endif

