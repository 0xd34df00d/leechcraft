#ifndef LMP_H
#define LMP_H
#include <memory>
#include <QWidget>
#include <QTranslator>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <interfaces/icustomprovider.h>
#include "ui_tabwidget.h"

class QToolBar;

class LMP : public QWidget
		  , public IInfo
		  , public IEmbedTab
		  , public ICustomProvider
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab ICustomProvider)

	Ui::TabWidget Ui_;
	std::auto_ptr<QTranslator> Translator_;
	std::auto_ptr<QAction> Open_;
	std::auto_ptr<QAction> Play_;
	std::auto_ptr<QAction> Pause_;
	std::auto_ptr<QAction> ViewerSettings_;
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
	QWidget* GetTabContents ();

	bool ImplementsFeature (const QString&) const;
private:
	QToolBar* SetupToolbar ();
	void ApplyVideoSettings (qreal, qreal, qreal, qreal);
public slots:
	void handleStateUpdated (const QString&);
	void handleError (const QString&);
	void setFile (const QString&);
	void play ();
private slots:
	void selectFile ();
	void changeViewerSettings ();
signals:
	void bringToFront ();
};

#endif

