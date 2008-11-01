#ifndef LMP_H
#define LMP_H
#include <memory>
#include <QWidget>
#include <QTranslator>
#include <QAction>
#include <interfaces/interfaces.h>
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
	QToolBar* SetupToolbar ();
	bool ImplementsFeature (const QString&) const;
public slots:
	void handleStateUpdated (const QString&);
	void setFile (const QString&);
	void play ();
private slots:
	void selectFile ();
};

#endif

