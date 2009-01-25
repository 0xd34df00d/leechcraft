#ifndef REMOTER_H
#define REMOTER_H
#include <QMainWindow>
#include <interfaces/iinfo.h>
#include <interfaces/iwindow.h>
#include "ui_mainwindow.h"

namespace LeechCraft
{
	namespace Util
	{
		class MergeModel;
	};
};

class Remoter : public QMainWindow
              , public IInfo
              , public IWindow
{
    Q_OBJECT
    Q_INTERFACES (IInfo IWindow);

    Ui::MainWindow Ui_;
    bool IsShown_;
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
public Q_SLOTS:
    void handleHidePlugins ();
	void pushHistoryModel (LeechCraft::Util::MergeModel*) const;
	void pushDownloadersModel (LeechCraft::Util::MergeModel*) const;
};

#endif

