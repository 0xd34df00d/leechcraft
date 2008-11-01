#ifndef LMP_H
#define LMP_H
#include <memory>
#include <QWidget>
#include <QTranslator>
#include <interfaces/interfaces.h>
#include "ui_tabwidget.h"

class LMP : public QWidget
		  , public IInfo
		  , public IEmbedTab
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab)

	Ui::TabWidget Ui_;
	std::auto_ptr<QTranslator> Translator_;
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
	QWidget *GetTabContents ();
public slots:
	void handleStateUpdated (const QString&);
};

#endif

