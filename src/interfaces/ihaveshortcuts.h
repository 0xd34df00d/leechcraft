#ifndef IHAVESHORTCUTS_H
#define IHAVESHORTCUTS_H
#include <QtPlugin>
#include <QMultiMap>
#include <QString>
#include <QKeySequence>
#include <QIcon>

class QAction;

namespace LeechCraft
{
	struct ActionInfo
	{
		QString UserVisibleText_;
		QKeySequence Default_;
		QIcon Icon_;

		ActionInfo ()
		{
		}

		ActionInfo (const QString& uvt,
				QKeySequence seq, const QIcon& icon)
		: UserVisibleText_ (uvt)
		, Default_ (seq)
		, Icon_ (icon)
		{
		}
	};
};

Q_DECLARE_METATYPE (LeechCraft::ActionInfo);

class IShortcutProxy
{
public:
	virtual QKeySequence GetShortcut (const QObject*, const QString&) const = 0;
	virtual ~IShortcutProxy () { }
};

class IHaveShortcuts
{
public:
	virtual void SetShortcutProxy (const IShortcutProxy*) = 0;
	virtual void SetShortcut (const QString&, const QKeySequence&) = 0;
	virtual QMap<QString, LeechCraft::ActionInfo> GetActionInfo () const = 0;
	virtual ~IHaveShortcuts () { }
};

Q_DECLARE_INTERFACE (IShortcutProxy, "org.Deviant.LeechCraft.IShortcutProxy/1.0");
Q_DECLARE_INTERFACE (IHaveShortcuts, "org.Deviant.LeechCraft.IHaveShortcuts/1.0");

#endif

