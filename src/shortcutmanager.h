#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H
#include <QDialog>
#include "ui_shortcutmanager.h"
#include "interfaces/ihaveshortcuts.h"

namespace LeechCraft
{
	class ShortcutManager : public QWidget
						  , public IShortcutProxy
	{
		Q_OBJECT
		Q_INTERFACES (IShortcutProxy);

		Ui::ShortcutManager Ui_;
		enum
		{
			RoleObject = 50,
			RoleOriginalName,
			RoleSequence,
			RoleOldSequence
		};
	public:
		ShortcutManager (QWidget* = 0);
		void AddObject (QObject*);
		void AddObject (QObject*, const QString&,
				const QString&, const QIcon&);
		QKeySequence GetShortcut (const QObject*, int) const ;
	public slots:
		void on_Tree__itemActivated (QTreeWidgetItem*);
		virtual void accept ();
		virtual void reject ();
	};
};

#endif

