/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <memory>
#include <QtPlugin>

namespace LeechCraft
{
namespace Monocle
{
	class IFormField;

	typedef std::shared_ptr<IFormField> IFormField_ptr;
	typedef QList<IFormField_ptr> IFormFields_t;

	class ISupportForms
	{
	public:
		virtual ~ISupportForms () {}

		virtual IFormFields_t GetFormFields (int page) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::ISupportForms,
		"org.LeechCraft.Monocle.ISupportForms/1.0");
