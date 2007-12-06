#ifndef UPDATERPLUGIN_H
#define UPDATERPLUGIN_H
#include <QMainWindow>
#include <interfaces/interfaces.h>

class SettingsDialog;
class QTreeWidget;
class QToolBar;
class Core;

class UpdaterPlugin : public QMainWindow
					, public IInfo
					, public IWindow
					, public IVersionable
{
	Q_OBJECT
	Q_INTERFACES (IInfo IWindow IVersionable);

	bool IsShown_;
	ID_t ID_;
	SettingsDialog *SettingsDialog_;
	QTreeWidget *Updates_;
	QToolBar *MainToolbar_;
	QAction *CheckForUpdates_, *Settings_;
	bool SaveChangesScheduled_;

	enum Columns
	{
		ColumnName_ = 0
		, ColumnLocation_ = 1
	};

	Core *Core_;
public:
	virtual void Init ();
	virtual ~UpdaterPlugin ();
	virtual QString GetName () const;
	virtual QString GetInfo () const;
	virtual QString GetStatusbarMessage () const;
	virtual IInfo& SetID (ID_t);
	virtual ID_t GetID () const;
	virtual QStringList Provides () const;
	virtual QStringList Needs () const;
	virtual QStringList Uses () const;
	virtual void SetProvider (QObject*, const QString&);
	virtual void Release ();
	virtual QIcon GetIcon () const;
	virtual void SetParent (QWidget*);
	virtual void ShowWindow ();
	virtual void ShowBalloonTip ();
	virtual uint GetVersion () const;
private:
	void SetupInterface ();
	void SetupMainWidget ();
	void SetupToolbars ();
	void SetupActions ();
	void SetupMenus ();
	void ReadSettings ();
private slots:
	void saveSettings ();
	void showSettings ();
	void addFile (const QString&, const QString&, const QString&);
	void handleError (const QString&);
};

#endif

