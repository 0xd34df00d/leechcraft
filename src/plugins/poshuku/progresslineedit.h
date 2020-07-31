/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PROGRESSLINEEDIT_H
#define PLUGINS_POSHUKU_PROGRESSLINEEDIT_H
#include <QKeyEvent>
#include <QLineEdit>
#include "interfaces/poshuku/iaddressbar.h"

class QModelIndex;
class QToolBar;
class QToolButton;

namespace LC
{
namespace Poshuku
{
	class ProgressLineEdit : public QLineEdit
							, public IAddressBar
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::IAddressBar)

		bool IsCompleting_;
		QString PreviousUrl_;

		QToolButton *ClearButton_;

		QList<QToolButton*> VisibleButtons_;
		QList<QToolButton*> HideButtons_;
		QHash<QAction*, QToolButton*> Action2Button_;
	public:
		ProgressLineEdit (QWidget* = 0);

		bool IsCompleting () const;
		QObject* GetQObject ();

		int ButtonsCount () const;
		QToolButton* AddAction (QAction*, bool = false);
		QToolButton* InsertAction (QAction*, int pos = -1, bool = false);
		QToolButton* GetButtonFromAction (QAction*) const;
		void RemoveAction (QAction*);
		void SetVisible (QAction*, bool);
	protected:
		void keyPressEvent (QKeyEvent*);
		void resizeEvent (QResizeEvent*);
		void contextMenuEvent (QContextMenuEvent*);
	private slots:
		void handleCompleterActivated ();
		void textChanged (const QString& text);
		void RepaintButtons ();
		void handleTriggeredButton (QAction*);
		void pasteGo ();
	signals:
		void actionTriggered (QAction*, const QString&);
	};
}
}

#endif
