#ifndef CSTP_H
#define CSTP_H
#include <interfaces/interfaces.h>
#include "ui_cstp.h"

class XmlSettingsDialog;

class CSTP : public QMainWindow
			 , public IInfo
			 , public IWindow
{
	Q_OBJECT
	Q_INTERFACES (IInfo IWindow)

	Ui::CSTP Ui_;
	unsigned long int ID_;
	bool IsShown_;
	QMenu *Plugins_;
	XmlSettingsDialog *XmlSettingsDialog_;
public:
	explicit CSTP ();
	virtual ~CSTP ();
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
private slots:
	void on_ActionAddTask__triggered ();
	void on_ActionPreferences__triggered ();
};

#endif

