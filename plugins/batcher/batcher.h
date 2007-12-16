#ifndef BATCHER_H
#define BATCHER_H
#include <QMainWindow>
#include <interfaces/interfaces.h>

class Batcher : public QMainWindow
			  , public IInfo
			  , public IWindow
{
	Q_OBJECT;
	Q_INTERFACES (IInfo IWindow);

	ID_t ID_;
public:
	virtual void Init ();
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
};

#endif

