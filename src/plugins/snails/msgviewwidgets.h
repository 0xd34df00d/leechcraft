/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <functional>
#include <QTabWidget>
#include <QGroupBox>

class QModelIndex;

namespace LeechCraft
{
namespace Snails
{
	typedef std::function<QWidget* (QModelIndex)> WidgetCreator_f;

	class MultipartAlternativeWidget : public QTabWidget
	{
		Q_OBJECT
	public:
		MultipartAlternativeWidget (const QModelIndex&, WidgetCreator_f, QWidget* = 0);
	};

	class MultipartSignedWidget : public QGroupBox
	{
		Q_OBJECT
	public:
		MultipartSignedWidget (const QModelIndex&, WidgetCreator_f, QWidget* = 0);
	};

	class GenericMultipartWidget : public QGroupBox
	{
		Q_OBJECT
	public:
		GenericMultipartWidget (const QModelIndex&, WidgetCreator_f, QWidget* = 0);
	};

	class Message822Widget : public QGroupBox
	{
		Q_OBJECT
	public:
		Message822Widget (const QModelIndex&, WidgetCreator_f, QWidget* = 0);
	};
}
}
