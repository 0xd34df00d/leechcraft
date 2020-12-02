/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "guiconfig.h"

class QToolButton;
class QLineEdit;

namespace LC::Util
{
	/** @brief Manages additional overlay buttons in a QLineEdit.
	 *
	 * This class manages custom buttons (like "Clear") on a QLineEdit or
	 * a derived class. Particularly, it lays outs the buttons next to
	 * each other and updates their positions when the managed line edit
	 * is moved or resized.
	 *
	 * Only one LineEditButtonManager can be installed on a single
	 * QLineEdit. Installing more than one manager will result in an
	 * exception thrown from the constructor.
	 *
	 * The managed line edit owns the created LineEditButtonManager.
	 *
	 * @sa ClearLineEditAddon
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API LineEditButtonManager : public QObject
	{
		QLineEdit * const Edit_;
		const int FrameWidth_;

		int Pad_;

		QList<QToolButton*> Buttons_;
	public:
		/** @brief Constructs the manager for the line \em edit.
		 *
		 * Constructs the LineEditButtonManager managing the given line
		 * \em edit.
		 *
		 * \em edit becomes the owner of the manager.
		 *
		 * @param[in] edit The line edit to manage.
		 * @exception std::runtime_error the line edit is already managed
		 * by another LineEditButtonManager instance.
		 */
		explicit LineEditButtonManager (QLineEdit *edit);

		/** @brief Adds a \em button to the line edit.
		 *
		 * @param[in] button The button to add to the managed line edit.
		 */
		void Add (QToolButton *button);
	protected:
		bool eventFilter (QObject*, QEvent*) override;
	private:
		void UpdatePos ();
	};
}
