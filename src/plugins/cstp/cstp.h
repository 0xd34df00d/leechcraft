#ifndef CSTP_H
#define CSTP_H
#include <memory>
#include <interfaces/interfaces.h>

class XmlSettingsDialog;
class Core;
class QTabWidget;
class QToolBar;
class QModelIndex;
class QTranslator;

namespace boost
{
	namespace logic
	{
		class tribool;
	};
};

namespace Ui
{
	class TabWidget;
};

class CSTP : public QObject
			 , public IInfo
			 , public IDownload
			 , public IJobHolder
{
	Q_OBJECT
	Q_INTERFACES (IInfo IDownload IJobHolder)

	QMenu *Plugins_;
	std::auto_ptr<QTranslator> Translator_;
	std::auto_ptr<XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<Ui::TabWidget> UiTabWidget_;
	std::auto_ptr<QTabWidget> TabWidget_;
	std::auto_ptr<QToolBar> Toolbar_;
public:
	virtual ~CSTP ();
	void Init ();
	void Release ();
    QString GetName () const;
    QString GetInfo () const;
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    QIcon GetIcon () const;

	qint64 GetDownloadSpeed () const;
	qint64 GetUploadSpeed () const;
	void StartAll ();
	void StopAll ();
	bool CouldDownload (const QByteArray&, LeechCraft::TaskParameters) const;
	int AddJob (const LeechCraft::DownloadParams&, LeechCraft::TaskParameters);

	QAbstractItemModel* GetRepresentation () const;
	LeechCraft::Util::HistoryModel* GetHistory () const;
	QWidget* GetControls () const;
	QWidget* GetAdditionalInfo () const;
	void ItemSelected (const QModelIndex&);
public slots:
	void on_ActionRemoveItemFromHistory__triggered ();
	void showSettings (int = -1);
private:
	template<typename T> void ApplyCore2Selection (void (Core::*) (const QModelIndex&), T);
	void SetupTabWidget ();
	void SetupToolbar ();
private slots:
	void handleError (const QString&);
	void handleFileExists (boost::logic::tribool*);
signals:
	void jobFinished (int);
	void jobRemoved (int);
	void jobError (int, IDownload::Error);
	void fileDownloaded (const QString&);
	void downloadFinished (const QString&);
};

#endif

