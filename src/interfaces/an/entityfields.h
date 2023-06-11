/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

/** @brief Namespace for various AN entity fields.
 *
 * This namespace contains various constants for widely-used fields
 * in an AdvancedNotifications-related Entity structure. Both event
 * notification fields and rule creation fields are mentioned.
 */
namespace LC::AN::EF
{
	/** @brief The plugin ID of the sender (QByteArray or QString).
	 *
	 * @note This field is required for event notification entities.
	 *
	 * @note This field is required for rule creation entities and should
	 * be equal to <em>org.LC.AdvNotifications.RuleRegister</em>.
	 */
	Q_DECL_IMPORT extern const QString SenderID;

	/** @brief The category of the event (QString).
	 *
	 * To notify about an event, this field should contain one of the
	 * predefined event categories (like CatIM, CatDownloads and so on).
	 * To cancel an event (for example, when all unread messages have
	 * been read), this field should contain the #CatEventCancel
	 * category.
	 *
	 * @note This field is required for event notification entities.
	 *
	 * @note This field is required for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString EventCategory;

	/** @brief The ID of the event (QString).
	 *
	 * Events relating to the same object (like IM messages from the same
	 * contact) should have the same event ID.
	 *
	 * @note This field is required for event notification entities.
	 *
	 * @note This field is required for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString EventID;

	/** @brief Visual path to this event (QStringList).
	 *
	 * This field should contain the list of human-readable strings that
	 * allow grouping of various events into tree-like structures.
	 *
	 * @note This field is required for event notification entities.
	 */
	Q_DECL_IMPORT extern const QString VisualPath;

	/** @brief The type of the event (QString).
	 *
	 * This field should contain one of the event types related to the
	 * given EventCategory, like TypeIMAttention or TypeIMIncMsg for
	 * the CatIM category.
	 *
	 * @note This field is also used when creating rules. In this case,
	 * it should contain a QStringList with all the event types the rule
	 * being created relates to.
	 *
	 * @note This field is required for event notification entities.
	 *
	 * @note This field is required for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString EventType;

	/** @brief The detailed text of the event (QString).
	 *
	 * @note This field is optional for event notification entities.
	 */
	Q_DECL_IMPORT extern const QString FullText;

	/** @brief The even more detailed text than FullText (QString).
	 *
	 * @note This field is optional for event notification entities.
	 */
	Q_DECL_IMPORT extern const QString ExtendedText;

	/** @brief The change in event count (int).
	 *
	 * This field represents the change in the count of the events with
	 * the given EventID.
	 *
	 * For example, if two messages arrive simultaneously from the same
	 * contact in an IM client, this field should be equal to 2.
	 *
	 * @note Either this field or the Count field should be present for
	 * event notification entities.
	 */
	Q_DECL_IMPORT extern const QString DeltaCount;

	/** @brief The new total event count (int).
	 *
	 * This field represents how many events with the given EventID are
	 * there pending now.
	 *
	 * @note Either this field or the DeltaCount field should be present
	 * for event notification entities.
	 */
	Q_DECL_IMPORT extern const QString Count;

	/** @brief The index of the window associated with this event (int).
	 *
	 * This field contains the index of the window containing the tab
	 * (or other widget) associated with the event.
	 * For example, for an IM notification, this could be the window
	 * containing the tab with the conversation in question.
	 */
	Q_DECL_IMPORT extern const QString WindowIndex;

	/** @brief Whether configuration dialog should be opened (bool).
	 *
	 * If this field is set to <code>true</code>,
	 *
	 * @note This field is optional for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString OpenConfiguration;

	/** @brief Whether the created rule should be single-shot (bool).
	 *
	 * @note This field is optional for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString IsSingleShot;

	/** @brief Whether a transient notifier should be enabled by default
	 * in the rule being created (bool).
	 *
	 * @note This field is optional for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString NotifyTransient;

	/** @brief Whether a persistent notifier should be enabled by default
	 * in the rule being created (bool).
	 *
	 * @note This field is optional for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString NotifyPersistent;

	/** @brief Whether an audio notifier should be enabled by default in
	 * the rule being created (bool).
	 *
	 * @note This field is optional for rule creation entities.
	 */
	Q_DECL_IMPORT extern const QString NotifyAudio;

	Q_DECL_IMPORT extern const QString RuleID;
	Q_DECL_IMPORT extern const QString AssocColor;
	Q_DECL_IMPORT extern const QString IsEnabled;
}

namespace LC
{
	Q_DECL_IMPORT extern const QString IgnoreSelf;
	Q_DECL_IMPORT extern const QString AllowedSemantics;
}
