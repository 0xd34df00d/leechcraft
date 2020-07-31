/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_DELEGATEBUTTONGROUP_H
#define PLUGINS_LACKMAN_DELEGATEBUTTONGROUP_H
#include <QObject>

class QAbstractButton;

namespace LC
{
namespace LackMan
{
	/** @brief Button group with 0 or 1 checked buttons.
		*
		* Usual QButtonGroup doesn't allow to have no checked
		* buttons at all. This class is used to work around this
		* and allows one to have no checked buttons.
		*
		* Used in PackagesDelegate to represent the update or
		* (install|remove) action group.
		*/
	class DelegateButtonGroup : public QObject
	{
		Q_OBJECT

		QList<QAbstractButton*> Buttons_;
	public:
		DelegateButtonGroup(QObject* = 0);

		void AddButton (QAbstractButton*);
	private slots:
		void handleButtonToggled (bool);
	};
}
}

#endif
