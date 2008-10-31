#ifndef LMP_H
#define LMP_H
#include <QWidget>
#include <interfaces/interfaces.h>

class TabWidget;

class LMP : public QObject
		  , public IInfo
		  , public IEmbedTab
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab)

	TabWidget *TabWidget_;
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
};

#endif

