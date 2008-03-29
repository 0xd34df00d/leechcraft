#ifndef MAILLEECHER_H
#define MAILLEECHER_H
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include "ui_mailleecher.h"

class MailLeecher : public QMainWindow
                    , public IInfo
                    , public IWindow
{
    Q_OBJECT
    Q_INTERFACES (IInfo IWindow);

    Ui::MailLeecher Ui_;
    ulong ID_;
    bool IsShown_, IsLeeching_;
public:
    void Init ();
    void Release ();
    QString GetName () const;
    QString GetInfo () const;
    QString GetStatusbarMessage () const;
    IInfo& SetID (ulong);
    ulong GetID () const;
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
    virtual void closeEvent (QEvent*);
public slots:
    void handleHidePlugins ();
private slots:
    void on_ServerAddress__textChanged ();
    void on_OutputDirectory__textChanged ();
    void on_Login__textChanged ();
    void on_OutputBrowse__released ();
    void on_Leech__released ();
    void setActionsEnabled ();
    void showError (const QString&);
    void doLog (const QString&);
};


#endif

