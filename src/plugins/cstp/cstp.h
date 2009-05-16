#ifndef CSTP_H
#define CSTP_H
#include <memory>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/structures.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

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

class CSTP : public QObject
		   , public IInfo
		   , public IDownload
		   , public IJobHolder
		   , public IHaveSettings
{
	Q_OBJECT
	Q_INTERFACES (IInfo IDownload IJobHolder IHaveSettings)

	QMenu *Plugins_;
	std::auto_ptr<QTranslator> Translator_;
	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<QToolBar> Toolbar_;
public:
	virtual ~CSTP ();
	void Init (ICoreProxy_ptr);
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
	bool CouldDownload (const LeechCraft::DownloadEntity&) const;
	int AddJob (LeechCraft::DownloadEntity);

	QAbstractItemModel* GetRepresentation () const;
	LeechCraft::Util::HistoryModel* GetHistory () const;
	void ItemSelected (const QModelIndex&);

	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;
private:
	template<typename T> void ApplyCore2Selection (void (Core::*) (const QModelIndex&), T);
	void SetupTabWidget ();
	void SetupToolbar ();
private slots:
	void handleFileExists (boost::logic::tribool*);
signals:
	void jobFinished (int);
	void jobRemoved (int);
	void jobError (int, IDownload::Error);
	void gotEntity (const LeechCraft::DownloadEntity&);
	void downloadFinished (const QString&);
	void log (const QString&);
};

#endif

