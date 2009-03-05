#ifndef SEEKTHRU_H
#define SEEKTHRU_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/structures.h>

class SeekThru : public QObject
			   , public IInfo
			   , public IFinder
			   , public IHaveSettings
{
	Q_OBJECT
	Q_INTERFACES (IInfo IFinder IHaveSettings)

	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
public:
	void Init ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QIcon GetIcon () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);

	QStringList GetCategories () const;
	boost::shared_ptr<IFindProxy> GetProxy (const LeechCraft::Request&);

	boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;
private slots:
	void handleError (const QString&);
signals:
	void delegateEntity (const LeechCraft::DownloadEntity&,
			int*, QObject**);
};

#endif

