/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <QModelIndex>
#include <QtPlugin>
#include "interfaces/structures.h"

class QAbstractItemModel;
class QMenu;
class QToolBar;

template<typename>
class QList;

namespace LC
{
	enum class ProcessKind : std::uint8_t
	{
		Download,
		Upload,
		Generic,
	};

	/** @brief Describes a process represented by a row in an IJobHolder model.
	 *
	 * This is one of the alternatives of the SpecificInfo variant stored
	 * in RowInfo::Specific_, used for rows representing ongoing processes
	 * (downloads, uploads, file transfers, etc.).
	 *
	 * @sa IJobHolder
	 * @sa RowInfo
	 */
	struct ProcessInfo
	{
		TaskParameters Parameters_ {};

		ProcessKind Kind_;

		bool operator== (const ProcessInfo& other) const = default;
	};

	struct NewsInfo
	{
		qlonglong Count_ = 0;
		QDateTime LastUpdate_;

		bool operator== (const NewsInfo& other) const = default;
	};

	using SpecificInfo = std::variant<
			ProcessInfo,
			NewsInfo
		>;

	struct RowInfo
	{
		QString Name_;
		SpecificInfo Specific_;

		bool operator== (const RowInfo& other) const = default;
	};

	enum class ProcessState : std::uint8_t
	{
		Running,
		Paused,
		Finished,
		Error,
		Unknown,
	};

	/** @brief This enum contains roles that are used to query job states.
	 */
	enum class JobHolderRole
	{
		/** This role is for the LC::RowInfo struct.
		 *
		 * The value at this role is a `RowInfo`.
		 */
		RowInfo = MaxValue<CustomDataRoles> + 1,
	};

	constexpr int operator+ (JobHolderRole role) noexcept
	{
		return static_cast<int> (role);
	}

	template<>
	inline constexpr int MaxValue<JobHolderRole> = +JobHolderRole::RowInfo;

	enum class JobHolderProcessRole
	{
		Done = MaxValue<JobHolderRole> + 1, // qint64
		Total, // qint64
		ProgressCustomText, // QString
		State, // ProcessState
		StateCustomText, // QString
	};

	constexpr int operator+ (JobHolderProcessRole role) noexcept
	{
		return static_cast<int> (role);
	}

	template<>
	inline constexpr int MaxValue<JobHolderProcessRole> = +JobHolderProcessRole::StateCustomText;
}

class IJobHolderRepresentationHandler
{
public:
	virtual ~IJobHolderRepresentationHandler () = default;

	/** @brief Returns the item representation model.
	 *
	 * The returned model is role-based: each row should provide a
	 * RowInfo via JobHolderRole::RowInfo, and process rows should
	 * also provide JobHolderProcessRole values (Done, Total, State,
	 * StateCustomText). Inside of LeechCraft the model would be
	 * merged with the models of other handlers, both of this and of
	 * other plugins.
	 *
	 * This model is also used to retrieve controls and additional info
	 * for a given index via the CustomDataRoles::RoleControls and
	 * CustomDataRoles::RoleAdditionalInfo respectively.
	 *
	 * Returned controls widget would be placed above the view with the
	 * jobs, so usually it has some actions controlling the job, but in
	 * fact it can have anything you want. It is only visible when a job
	 * from your plugin is selected. If a job from other plugin is
	 * selected, then other plugin's controls would be placed, and if no
	 * jobs are selected at all then all controls are hidden.
	 *
	 * Widget with the additional information is placed to the right of
	 * the view with the jobs, so usually it has additional information
	 * about the job like transfer log for FTP client, but in fact it
	 * can have anything you want. The same rules regarding its
	 * visibility apply as for controls widget.
	 *
	 * @return Representation model.
	 *
	 * @sa IJobHolder
	 * @sa LC::CustomDataRoles
	 * @sa LC::RowInfo
	 * @sa LC::JobHolderProcessRole
	 */
	virtual QAbstractItemModel& GetRepresentation () = 0;

	/** @brief The rows of this handler's representation selected in the view.
	 */
	struct RowSelection
	{
		QList<QModelIndex> Rows_;		///< The selected rows, at the 0'th column, in the view's order.
		QModelIndex Current_;			///< One of Rows_ (at the 0'th column), or invalid.

		static RowSelection FromMaybe (const std::optional<QModelIndex>& row)
		{
			if (!row)
				return {};
			return FromSingle (*row);
		}

		static RowSelection FromSingle (const QModelIndex& row)
		{
			return { .Rows_ { row }, .Current_ { row } };
		}
	};

	/** @brief Called from a clean stack whenever the selection or the
	 * current row changes.
	 *
	 * Changes are coalesced and reported before the next repaint, but
	 * possibly while a mouse gesture (like a click or a drag selection) is
	 * still in progress, so the implementation may update its own state
	 * and widgets, but must not modify the representation model's
	 * structure: the rows would shift under the pressed pointer.
	 *
	 * @param[in] selection The selected rows, empty if none of this
	 * handler's rows are selected anymore.
	 */
	virtual void HandleSelectedRowsChanging ([[maybe_unused]] const RowSelection& selection) {}

	/** @brief Called from a clean stack once the selection has settled.
	 *
	 * Multiple changes are coalesced, and this is never invoked while the
	 * left mouse button is held on the view. The implementation may modify
	 * the representation model, for example, hide the rows deselected by
	 * the user.
	 *
	 * @param[in] selection The selected rows, as in
	 * HandleSelectedRowsChanging().
	 */
	virtual void HandleSelectedRowsSettled ([[maybe_unused]] const RowSelection& selection) {}

	// Invoked synchronously from the corresponding QAbstractItemView signals.
	virtual void HandleActivated (const QModelIndex&) {}
	virtual void HandleClicked (const QModelIndex&) {}
	virtual void HandleDoubleClicked (const QModelIndex&) {}
	virtual void HandlePressed (const QModelIndex&) {}

	virtual QWidget* GetInfoWidget () { return nullptr; }
	virtual QToolBar* GetControls () { return nullptr; }
	virtual QMenu* GetContextMenu () { return nullptr; }
};

using IJobHolderRepresentationHandler_ptr = std::unique_ptr<IJobHolderRepresentationHandler>;

/** @brief Interface for plugins holding jobs or persistent notifications.
 *
 * If a plugin can have some long-performing jobs (like a BitTorrent
 * download, or file transfer in an IM client, or mail checking status)
 * or persistent notifications (like unread messages in an IM client,
 * unread news in an RSS feed reader or weather forecast), it may want
 * to implement this interface to display itself in plugins like
 * Summary.
 *
 * The models with jobs and state info, as well as the controls and the
 * information panes for them, are provided by the handlers created via
 * CreateRepresentationHandlers(). The rows of the models carry some
 * metadata like job progress via the JobHolderRole and
 * JobHolderProcessRole enumerations (see RowInfo for an example).
 *
 * Controls and additional information pane of a handler are only
 * visible when a job of that handler is selected.
 *
 * @sa IJobHolderRepresentationHandler
 * @sa IDownload
 * @sa CustomDataRoles
 * @sa JobHolderRole
 * @sa JobHolderProcessRole
 * @sa RowInfo
 */
class Q_DECL_EXPORT IJobHolder
{
protected:
	virtual ~IJobHolder () = default;
public:
	/** @brief The callbacks into the view showing the handlers' representations.
	 */
	struct ViewCallbacks
	{
		/** @brief Selects the given rows of the handler's representation.
		 *
		 * Replaces the view's selection with those of the given rows the
		 * view shows, possibly none, making RowSelection::Current_ the
		 * current one.
		 *
		 * The resulting selection is reported via the handler's hooks from
		 * a clean stack, never before this returns. Shall not be called
		 * from within CreateRepresentationHandlers(): the view is not set
		 * up yet by then.
		 */
		std::function<void (IJobHolderRepresentationHandler::RowSelection)> SetSelection_;
	};

	/** @brief Creates the handlers representing the jobs of this plugin.
	 *
	 * Each handler is presented as if it came from a separate plugin:
	 * its model is merged with the others in the returned order, and its
	 * controls, info widget, context menu and selection hooks are the
	 * ones used while a row of its model is current or selected. Thus a
	 * plugin whose jobs come in kinds needing different controls, like
	 * unread channels and running feed updates, returns a handler per
	 * kind.
	 *
	 * The models of the returned handlers shall be distinct, as the views
	 * merge them by identity. A model may still be shared with the
	 * handlers returned to other views: this function is invoked once per
	 * view showing the jobs, and each view keeps the handlers it got for
	 * as long as it exists.
	 *
	 * @param[in] callbacks The callbacks into the view the handlers are
	 * created for.
	 * @return The handlers, possibly none.
	 *
	 * @sa MakeHandlers()
	 * @sa IJobHolderRepresentationHandler
	 */
	virtual std::vector<IJobHolderRepresentationHandler_ptr> CreateRepresentationHandlers (const ViewCallbacks& callbacks) = 0;
protected:
	/** @brief Wraps the given handlers into the list to be returned from
	 * CreateRepresentationHandlers().
	 *
	 * A braced list can't do that: std::initializer_list only ever yields
	 * const elements, while the handlers are move-only.
	 */
	template<std::derived_from<IJobHolderRepresentationHandler>... Handlers>
	static std::vector<IJobHolderRepresentationHandler_ptr> MakeHandlers (std::unique_ptr<Handlers>... handlers)
	{
		std::vector<IJobHolderRepresentationHandler_ptr> result;
		result.reserve (sizeof... (Handlers));
		(result.push_back (std::move (handlers)), ...);
		return result;
	}
};

Q_DECLARE_METATYPE (LC::RowInfo)
Q_DECLARE_METATYPE (LC::ProcessState)

Q_DECLARE_INTERFACE (IJobHolder, "org.Deviant.LeechCraft.IJobHolder/1.0")
