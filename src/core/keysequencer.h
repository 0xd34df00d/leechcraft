/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef KEYSEQUENCER_H
#define KEYSEQUENCER_H
#include <QDialog>
#include "ui_keysequencer.h"

class KeySequencer : public QDialog
{
	Q_OBJECT

	Ui::KeySequencer Ui_;
	QKeySequence Result_;

	int LastCode_;
public:
	KeySequencer (const QString&, QWidget* = 0);
	QKeySequence GetResult () const;
protected:
	virtual void keyPressEvent (QKeyEvent*);
	virtual void keyReleaseEvent (QKeyEvent*);
};

#endif

