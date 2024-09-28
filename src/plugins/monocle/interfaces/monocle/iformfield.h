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
#include "ilink.h"

namespace LC::Monocle
{
	/** @brief Describes the possible types of a form field.
	 *
	 * @sa IFormField
	 */
	enum class FormType
	{
		/** @brief A text entry field.
		 *
		 * Fields of this type should also implement IFormFieldText.
		 *
		 * @sa IFormFieldText
		 */
		Text,

		/** @brief A single- and multiple choice field.
		 *
		 * Fields of this type should also implement IFormFieldChoice.
		 *
		 * @sa IFormFieldChoice
		 */
		Choice,

		/** @brief A push button, radio button or check box.
		 *
		 * Fields of this type should also implement IFormFieldButton.
		 *
		 * @sa IFormFieldButton
		 */
		Button
	};

	/** @brief Base interface to be implemented by form fields.
	 *
	 * This is a base interface that should be implemented by all form
	 * fields disregarding their type. Depending on the type of this
	 * field returned by the GetType() method the corresponding class
	 * should also implement other interfaces. See ::FormType documentation
	 * for details.
	 *
	 * As a general rule, there is no "Apply" method in form interfaces.
	 * Instead, changes should be applied as soon as corresponding
	 * interface's setter method is called.
	 *
	 * @sa IFormFieldText, IFormFieldChoice, IFormFieldButton
	 */
	class IFormField
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IFormField () = default;

		/** @brief Returns the type of this field.
		 *
		 * @return The type of this field.
		 */
		virtual FormType GetType () const = 0;

		/** @brief Returns the unique ID of this field.
		 *
		 * The ID should be unique across the whole document.
		 *
		 * @return The unique ID of this field.
		 */
		virtual int GetID () const = 0;

		/** @brief Returns the user-visible name of this field.
		 *
		 * @return The human-readable name of this field.
		 */
		virtual QString GetName () const = 0;

		/** @brief Returns the rectangle this field occupies.
		 *
		 * All fields occupy a fixed area on the page and cannot grow or
		 * shrink depending on their content. This method returns the
		 * rectangle of the area that this field should occupy.
		 *
		 * @return The rectangle this field occupies in relative page
		 * coordinates.
		 */
		virtual PageRelativeRectBase GetRect () const = 0;

		/** @brief Returns the alignment of the contents of this field.
		 *
		 * This method returns, for example, text alignment in a text
		 * entry field or options alignment in a list widget.
		 *
		 * @return The alignment of the contents of this field.
		 */
		virtual Qt::Alignment GetAlignment () const = 0;
	};

	/** @brief A shared pointer to a IFormField.
	 */
	typedef std::shared_ptr<IFormField> IFormField_ptr;

	/** @brief Interface to be implemented by text fields.
	 *
	 * If a field is of type FormType::Text, it should also implement
	 * this interface.
	 *
	 * @sa IFormField
	 */
	class IFormFieldText
	{
	public:
		/** @brief Describes various types of text entry fields.
		 *
		 * @sa GetTextType()
		 */
		enum class Type
		{
			/** @brief Single line text edit.
			 */
			SingleLine,

			/** @brief Multiline text edit.
			 */
			Multiline,

			/** @brief File entry widget.
			 */
			File
		};

		/** @brief Virtual destructor.
		 */
		virtual ~IFormFieldText () = default;

		/** @brief Returns the current text value of this field.
		 *
		 * @return The current text value of this field.
		 *
		 * @sa SetText()
		 */
		virtual QString GetText () const = 0;

		/** @brief Sets the current text value of this field to \em text.
		 *
		 * @param[in] text The new text value of this field.
		 *
		 * @sa GetText()
		 */
		virtual void SetText (const QString& text) = 0;

		/** @brief Returns the exact type of this text entry field.
		 *
		 * @return The exact type of this text field.
		 */
		virtual Type GetTextType () const = 0;

		/** @brief Returns the maximum length of the text.
		 *
		 * A return value of 0 or less means the text length is
		 * unlimited.
		 *
		 * @return The maximum length of the text or non-positive value
		 * for unlimited length.
		 */
		virtual int GetMaximumLength () const = 0;

		/** @brief Returns whether this is a password entry field.
		 *
		 * Characters in password entry fields are masked by dots or
		 * something similar so that the entered text isn't visible.
		 *
		 * @return Whether this is a password entry field.
		 */
		virtual bool IsPassword () const = 0;

		/** @brief Returns whether rich text should be accepted.
		 *
		 * This only makes sense for Type::Multiline fields.
		 *
		 * @return Whether rich text should be accepted.
		 */
		virtual bool IsRichText () const = 0;
	};

	/** @brief Interface to be implemented by choice fields.
	 *
	 * If a field is of type FormType::Choice, it should also implement
	 * this interface.
	 *
	 * @sa IFormField
	 */
	class IFormFieldChoice
	{
	public:
		/** @brief Describes various types of choice fields.
		 *
		 * @sa GetChoiceType()
		 */
		enum class Type
		{
			/** @brief Combobox with choices.
			 *
			 * Only single choice is allowed.
			 *
			 * @sa GetEditChoice(), SetEditChoice(), IsEditable()
			 */
			Combobox,

			/** @brief List widget with choices.
			 *
			 * Multiple choices are possibly allowed.
			 *
			 * @sa GetCurrentChoices(), SetCurrentChoices()
			 */
			ListBox
		};

		/** @brief Virtual destructor.
		 */
		virtual ~IFormFieldChoice () = default;

		/** @brief Returns the exact type of this choice field.
		 *
		 * @return The exact type of this choice field.
		 */
		virtual Type GetChoiceType () const = 0;

		/** @brief Returns all available choices.
		 *
		 * This method returns the list of all available choices for both
		 * Type::Combobox and Type::ListBox choice fields.
		 *
		 * Please note that user can enter his own choice variant for
		 * Type::Combobox fields that are editable.
		 *
		 * @return All available choices.
		 *
		 * @sa GetCurrentChoices(), GetEditChoice()
		 */
		virtual QStringList GetAllChoices () const = 0;

		/** @brief Returns the list of current choices for a listbox.
		 *
		 * This method returns the currently selected variants in a
		 * Type::ListBox choice field. It does nothing for other types of
		 * fields.
		 *
		 * The numbers in the returned list are the indexes of the
		 * choices in the array returned by GetAllChoices().
		 *
		 * @return The indexes of the currently selected variants.
		 *
		 * @sa SetCurrentChoices(), GetAllChoices()
		 */
		virtual QList<int> GetCurrentChoices () const = 0;

		/** @brief Sets the currently selected choices for a listbox.
		 *
		 * This method sets the currently selected variants in a
		 * Type::ListBox choice field. It does nothing for other types of
		 * fields.
		 *
		 * The numbers in the \em choices list are the indexes of the
		 * choices in the array returned by GetAllChoices().
		 *
		 * @sa GetCurrentChoices(), GetAllChoices()
		 */
		virtual void SetCurrentChoices (const QList<int>& choices) = 0;

		/** @brief Returns the current choice for a combobox.
		 *
		 * This method returns the current choice in a Type::Combobox
		 * choice field. The returned value can be either one of the
		 * variants in the list returned by GetAllChoices() or it can be
		 * a custom text if this combobox is editable (that is,
		 * IsEditable() returns true).
		 *
		 * For other types of choice fields this method does nothing.
		 *
		 * @return The current choice, either predefined one or a custom
		 * one.
		 *
		 * @sa SetEditChoice(), IsEditable()
		 */
		virtual QString GetEditChoice () const = 0;

		/** @brief Sets the current choice for a combobox.
		 *
		 * This method updates the current choice in a Type::Combobox
		 * choice field. The \em choice can be either one of the variants
		 * in the list returned by GetAllChoices() or it can be a custom
		 * text if this combobox is editable (that is, IsEditable()
		 * returns true).
		 *
		 * For other types of choice fields this method does nothing.
		 *
		 * @param[in] choice The new choice, either predefined one or
		 * a custom one for an editable combobox.
		 *
		 * @sa GetEditChoice(), IsEditable()
		 */
		virtual void SetEditChoice (const QString& choice) = 0;

		/** @brief Returns whether this combobox is editable.
		 *
		 * This method returns whether this Type::Combobox choice field
		 * is editable — that is, whether custom user text can be passed
		 * to the SetEditChoice() method.
		 *
		 * For other types of fields this method doesn't make sense and
		 * always returns <code>false</code>.
		 *
		 * @return Whether this combobox is editable.
		 */
		virtual bool IsEditable () const = 0;
	};

	/** @brief Interface to be implemented by button fields.
	 *
	 * If a field is of type FormType::Button, it should also implement
	 * this interface.
	 *
	 * @sa IFormField.
	 */
	class IFormFieldButton
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IFormFieldButton () = default;

		/** @brief Describes various types of button fields.
		 *
		 * @sa GetButtonType()
		 */
		enum class Type
		{
			/** @brief A button that can be pushed.
			 *
			 * @sa HandleActivated()
			 */
			Pushbutton,

			/** @brief A field that can be checked independently of
			 * others.
			 *
			 * @sa IsChecked(), SetChecked()
			 */
			Checkbox,

			/** @brief A field that can be checked with respect to the
			 * check state of others.
			 *
			 * @sa IsChecked(), SetChecked(), GetButtonGroup()
			 */
			Radiobutton
		};

		/** @brief Returns the exact type of this button field.
		 *
		 * @return The exact type of this button field.
		 */
		virtual Type GetButtonType () const = 0;

		/** @brief Returns the caption of this button.
		 *
		 * This method should return the user-visible text on the button
		 * which may be way different form IFormField::GetName().
		 *
		 * @return The human-readable caption of the button.
		 */
		virtual QString GetCaption () const = 0;

		/** @brief Returns whether the button is checked.
		 *
		 * This method doesn't make sense for Type::Pushbutton buttons.
		 *
		 * @return Whether this button is checked.
		 *
		 * @sa SetChecked()
		 */
		virtual bool IsChecked () const = 0;

		/** @brief Updates the check state of this button.
		 *
		 * If the type of this button is Type::Radiobutton and this radio
		 * button belongs to a group (GetButtonGroup() returns a
		 * non-empty list), setting the check state to <em>code</em> true
		 * leads to setting the check state of the previously checked
		 * radio button to false.
		 *
		 * This method doesn't make sense for Type::Pushbutton and thus
		 * does nothing for push buttons.
		 *
		 * @param[in] state The new check state of this radio button or
		 * check box.
		 *
		 * @sa IsChecked()
		 */
		virtual void SetChecked (bool state) = 0;

		/** @brief Returns the button group this button belongs to.
		 *
		 * This method returns the list of IDs (returned by the
		 * IFormField::GetID() method) that form the same button group,
		 * where only one button at a time can be checked.
		 *
		 * If an empty list is returned, it is interpreted as if the
		 * button doesn't belong to any button groups.
		 *
		 * The returned list should be sorted and contain the self ID
		 * (the ID of the field that returns the list).
		 *
		 * This method only makes sense for buttons of Type::Radiobutton.
		 *
		 * @return The list of IDs of buttons that form the same group,
		 * or an empty list of the button doesn't belong to any groups.
		 */
		virtual QList<int> GetButtonGroup () const = 0;

		/** @brief Returns the action associated with this button.
		 *
		 * This method is invoked by Monocle when the corresponding field
		 * is triggered. Currently it only means that a field of
		 * Type::Pushbutton is activated by the user, and this method is
		 * never invoked for other types of fields.
		 */
		virtual LinkAction GetActivationAction () const = 0;
	};
}
