/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

namespace LC
{
namespace Monocle
{
	class IFormField;

	typedef std::shared_ptr<IFormField> IFormField_ptr;
	typedef QList<IFormField_ptr> IFormFields_t;

	/** @brief Interface for documents supporting inline forms.
	 *
	 * If a document is of format that supports page forms that can be
	 * filled, it should implement this interface.
	 *
	 * It also makes sense to implement ISaveableDocument so that changes
	 * to the forms could be saved.
	 *
	 * There is no "Apply" method in either this interface or form field
	 * interfaces. Changes should be applied as soon as corresponding
	 * interface's setter method is called.
	 *
	 * @sa ISaveableDocument, IFormField
	 */
	class ISupportForms
	{
	public:
		virtual ~ISupportForms () = default;

		/** @brief Returns the list of fields for the given page.
		 *
		 * This function should return the list of form fields found on
		 * the given \em page, or an empty list if no fields are present.
		 *
		 * @param[in] page The zero-based index of the page to query.
		 * @return The list of form fields on the page.
		 */
		virtual IFormFields_t GetFormFields (int page) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::ISupportForms,
		"org.LeechCraft.Monocle.ISupportForms/1.0")
