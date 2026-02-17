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
#include "interfaces/structures.h"

class QAbstractItemModel;
class QMenu;
class QModelIndex;
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
	 * merged with other models from other plugins.
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

	virtual void HandleCurrentChanged (const QModelIndex&) {}
	virtual void HandleCurrentColumnChanged (const QModelIndex&) {}
	virtual void HandleCurrentRowChanged (const QModelIndex&) {}
	virtual void HandleSelectedRowsChanged (const QList<QModelIndex>&) {}

	virtual void HandleActivated (const QModelIndex&) {}
	virtual void HandleClicked (const QModelIndex&) {}
	virtual void HandleDoubleClicked (const QModelIndex&) {}
	virtual void HandleEntered (const QModelIndex&) {}
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
 * The model with jobs and state info is obtained via GetRepresentation(),
 * and various roles are used to retrieve controls and information pane
 * of the plugin from that model, as well as some metadata like job
 * progress (see JobHolderRole and JobHolderProcessRole enumerations,
 * and RowInfo for an example).
 *
 * Controls and additional information pane are only visible when a job
 * handled by the plugin is selected.
 *
 * @sa IDownloader
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
	virtual IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () = 0;
};

Q_DECLARE_METATYPE (LC::RowInfo)
Q_DECLARE_METATYPE (LC::ProcessState)

Q_DECLARE_INTERFACE (IJobHolder, "org.Deviant.LeechCraft.IJobHolder/1.0")
