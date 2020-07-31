/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef COMMONJOBADDER_H
#define COMMONJOBADDER_H
#include <QDialog>
#include "ui_commonjobadder.h"

namespace LC
{
	/** Dialog for adding tasks directly via LeechCraft. Has two fields,
	 * What and Where, corresponding to Entity_ and Location_ fields of
	 * Entity respectively.
	 */
	class CommonJobAdder : public QDialog,
						   private Ui::CommonJobAdder
	{
		Q_OBJECT
	public:
		/** Creates the dialog and sets the what/where values to
		 * previous ones.
		 *
		 * @param[in] parent The parent widget.
		 */
		CommonJobAdder (QWidget *parent = 0);

		/** Returns the value of What field.
		 *
		 * @return The What.
		 */
		QString GetString () const;
	private slots:
		/** Handles clicking the Browse button near What field. Pops up
		 * the QFileDialog::getOpenFileName dialog.
		 */
		void on_Browse__released ();
	};
};

#endif

