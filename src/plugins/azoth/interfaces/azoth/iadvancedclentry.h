/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IADVANCEDCLENTRY_H
#define PLUGINS_AZOTH_INTERFACES_IADVANCEDCLENTRY_H
#include "imucbookmarkeditorwidget.h"

namespace LC
{
namespace Azoth
{
	/** This interface defines some advanced actions and signals on
	 * contact list entries, like methods for drawing attention and
	 * such.
	 *
	 * Entries implementing this interface should, of course, implement
	 * plain ICLEntry as well.
	 *
	 * @sa ICLEntry
	 */
	class IAdvancedCLEntry
	{
	public:
		virtual ~IAdvancedCLEntry () {}

		/** This enum represents some advanced features that may or may
		 * be not supported by advanced CL entries.
		 */
		enum AdvancedFeature
		{
			/** This entry supports drawing attention.
			 */
			AFSupportsAttention = 0x0001
		};

		Q_DECLARE_FLAGS (AdvancedFeatures, AdvancedFeature)

		/** Returns the OR-ed combination of advanced features supported
		 * by this contact list entry.
		 *
		 * @return The advanced features supported by this entry.
		 */
		virtual AdvancedFeatures GetAdvancedFeatures () const = 0;

		/** @brief Requests attention of the user behind this entry.
		 *
		 * This method, if called, should send request for attention to
		 * this entry, if supported by the protocol. An optional text
		 * message may be added to the attention request.
		 *
		 * If variant is an empty string, the variant with the highest
		 * priority should be used.
		 *
		 * @param[in] text Optional accompanying text.
		 * @param[in] variant The entry variant to buzz, or a null
		 * string for variant with highest priority.
		 *
		 * @sa attentionDrawn()
		 */
		virtual void DrawAttention (const QString& text, const QString& variant) = 0;
	protected:
		/** @brief Notifies about attention request from this entry.
		 *
		 * This signal should be emitted by the entry whenever the user
		 * behind the entry decides to request our own attention.
		 *
		 * Depending on Azoth settings, the request may be displayed in
		 * some way or ignored completely.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] text Optional accompanying text.
		 * @param[out] variant Source variant of the entry that has
		 * requested our attention.
		 *
		 * @sa DrawAttention()
		 */
		virtual void attentionDrawn (const QString& text, const QString& variant) = 0;

		/** @brief Notifies that entry's geolocation has changed.
		 *
		 * The actual geolocation information could be obtained via
		 * ISupportGeolocation::GetUserGeolocationInfo() method.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose location has
		 * changed.
		 * @sa ISupportGeolocation
		 */
		virtual void locationChanged (const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IAdvancedCLEntry,
		"org.Deviant.LeechCraft.Azoth.IAdvancedCLEntry/1.0")

#endif
