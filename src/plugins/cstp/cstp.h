#ifndef CSTP_H
#define CSTP_H
#include <interfaces/interfaces.h>
#include "ui_cstp.h"

class XmlSettingsDialog;

class CSTP : public QMainWindow
			 , public IInfo
			 , public IWindow
			 , public IDownload
			 , public IDirectDownload
			 , public IJobHolder
{
	Q_OBJECT
	Q_INTERFACES (IInfo IWindow IDownload IDirectDownload IJobHolder)

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

	qint64 GetDownloadSpeed () const;
	qint64 GetUploadSpeed () const;
	void StartAll ();
	void StopAll ();
	bool CouldDownload (const QString&, LeechCraft::TaskParameters) const;
	int AddJob (const DirectDownloadParams&, LeechCraft::TaskParameters);

	QAbstractItemModel* GetRepresentation () const;
	QAbstractItemDelegate* GetDelegate () const;
protected:
    virtual void closeEvent (QCloseEvent*);
private:
	template<typename T, typename U> void ApplyCore2Selection (T, U);
public slots:
	void handleHidePlugins ();
private slots:
	void on_ActionAddTask__triggered ();
	void on_ActionRemoveTask__triggered ();
	void on_ActionStart__triggered ();
	void on_ActionStop__triggered ();
	void on_ActionRemoveAll__triggered ();
	void on_ActionStartAll__triggered ();
	void on_ActionStopAll__triggered ();
	void on_ActionRemoveItemFromHistory__triggered ();
	void on_ActionPreferences__triggered ();
	void handleError (const QString&);
signals:
	void jobFinished (int);
	void jobRemoved (int);
	void jobError (int, IDirectDownload::Error);
	void fileDownloaded (const QString&);
	void downloadFinished (const QString&);
};

#endif

