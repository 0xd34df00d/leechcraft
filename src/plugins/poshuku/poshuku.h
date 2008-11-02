#ifndef POSHUKU_H
#define POSHUKU_H
#include <memory>
#include <QAction>
#include <QTranslator>
#include <QWidget>
#include <interfaces/interfaces.h>
#include "ui_poshuku.h"

class Poshuku : public QWidget
			  , public IInfo
			  , public IEmbedTab
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab)

	Ui::Poshuku Ui_;
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
};

#endif

