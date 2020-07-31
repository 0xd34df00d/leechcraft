/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include "interfaces/structures.h"

class QAbstractItemModel;
class QWidget;

namespace LC
{
	/** @brief Describes the columns in a job holder model.
	 *
	 * A job holder model has a fixed number of columns, and this enum
	 * gives them meaningful names.
	 */
	enum JobHolderColumn
	{
		/** @brief The column with the name of the task, like a torrent
		 * name or an RSS feed name.
		 */
		JobName,

		/** @brief The column with the status of the task, like the
		 * download status or the unread items count of an RSS feed.
		 */
		JobStatus,

		/** @brief The column with the progress of the task, like the
		 * amount of data downloaded so far or last update.
		 */
		JobProgress
	};

	/** @brief Describes the semantics of a row in a job holder model.
	 *
	 * Values of this enum are used to describe the semantics of rows
	 * in the representation models.
	 *
	 * Values of this enum are expected to be obtained via the
	 * CustomDataRoles::RoleJobHolderRow role.
	 */
	enum JobHolderRow
	{
		/** This row corresponds to something that cannot be described
		 * by other enum members.
		 */
		Other,

		/** This row corresponds to a news item, say, in an RSS reader
		 * or a Twitter client.
		 */
		News,

		/** This row corresponds to a pending download like in a
		 * BitTorrent client or an HTTP downloader.
		 *
		 * If a row has this type, then it also has to have meaningful
		 * process state for the JobHolderRole::ProcessState role.
		 */
		DownloadProgress,

		/** This row corresponds to some process like sending a file in
		 * IM, unpacking an archive or checking for new mail.
		 *
		 * If a row has this type, then it also has to have meaningful
		 * process state for the JobHolderRole::ProcessState role.
		 */
		ProcessProgress
	};

	/** @brief State of a single process represented in a IJobHolder model.
	 *
	 * This structure describes the a process represented by a row in an
	 * IJobHolder model and should be returned via the
	 * JobHolderRole::ProcessState role.
	 *
	 * The value of the CustomDataRoles::RoleJobHolderRow role should be
	 * either JobHolderRow::DownloadProgress or
	 * JobHolderRow::ProcessProgress.
	 *
	 * @sa IJobHolder
	 * @sa JobHolderRow
	 */
	struct ProcessStateInfo
	{
		/** @brief The amount of items already processed or downloaded.
		 *
		 * This can be the number of already downloaded bytes in an HTTP
		 * client, a number of messages fetched in an email client, and
		 * so on.
		 */
		qlonglong Done_ = 0;

		/** @brief The total amount of items to be processed or downloaded.
		 *
		 * This can be the number of already downloaded bytes in an HTTP
		 * client, a number of messages fetched in an email client, and
		 * so on.
		 */
		qlonglong Total_ = 0;

		/** @brief The flags of the task as it was originally added to the
		 * downloader, if relevant.
		 *
		 * This field only makes sense if the relevant process is a
		 * download process, that is, if value of the
		 * CustomDataRoles::RoleJobHolderRow role is
		 * JobHolderRow::DownloadProgress.
		 */
		TaskParameters Params_ = {};

		/** @brief Describes the state of the process.
		 */
		enum class State
		{
			/** @brief Unknown state.
			 */
			Unknown,

			/** @brief The process is running just fine.
			 */
			Running,

			/** @brief The process is paused.
			 */
			Paused,

			/** @brief There was an error completing the process.
			 */
			Error
		} State_ = State::Unknown;

		/** @brief Default-constructs a process description.
		 */
		ProcessStateInfo () = default;

		/** @brief Constructs the description with the given values.
		 *
		 * @param[in] done The value for the Done_ variable.
		 * @param[in] total The value for the Total_ variable.
		 * @param[in] params The value for the Params_ variable.
		 *
		 * @sa Done_
		 * @sa Total_
		 * @sa Params_
		 */
		ProcessStateInfo (qlonglong done, qlonglong total, TaskParameters params)
		: ProcessStateInfo { done, total, params, State::Unknown }
		{
		}

		/** @brief Constructs the description with the given values
		 * and state.
		 *
		 * @param[in] done The value for the Done_ variable.
		 * @param[in] total The value for the Total_ variable.
		 * @param[in] params The value for the Params_ variable.
		 * @param[in] state The value for the State_ variable.
		 *
		 * @sa Done_
		 * @sa Total_
		 * @sa Params_
		 * @sa State_
		 */
		ProcessStateInfo (qlonglong done, qlonglong total, TaskParameters params, State state)
		: Done_ { done }
		, Total_ { total }
		, Params_ { params }
		, State_ { state }
		{
		}
	};

	/** @brief This enum contains roles that are used to query job states.
	 */
	enum JobHolderRole
	{
		/** @brief Describes the state of a process.
		 *
		 * This role should return a meaningful value for the
		 * JobHolderRow::DownloadProgress and
		 * JobHolderRow::ProcessProgress rows.
		 *
		 * The returned value should be a ProcessStateInfo structure.
		 *
		 * @sa ProcessStateInfo
		 */
		ProcessState = CustomDataRoles::RoleMAX + 1
	};
}

class IJobHolderRepresentationHandler;
using IJobHolderRepresentationHandler_ptr = std::shared_ptr<IJobHolderRepresentationHandler>;

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
 * progress (see JobHolderRole enumeration and ProcessStateInfo for an
 * example).
 *
 * Returned model should have three columns: name, state and status.
 * Controls and additional information pane are only visible when a job
 * handled by the plugin is selected.
 *
 * @sa IDownloader
 * @sa CustomDataRoles
 * @sa JobHolderRole::ProcessState
 * @sa ProcessStateInfo
 */
class Q_DECL_EXPORT IJobHolder
{
public:
	/** @brief Returns the item representation model.
	 *
	 * The returned model should have three columns, each for name,
	 * state and progress with speed. Inside of LeechCraft it would be
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
	 * @sa LC::CustomDataRoles
	 * @sa LC::JobHolderRow
	 * @sa LC::ProcessStateInfo
	 */
	virtual QAbstractItemModel* GetRepresentation () const = 0;

	virtual IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () { return {}; }

	/** @brief Virtual destructor.
	 */
	virtual ~IJobHolder () {}
};

Q_DECLARE_METATYPE (LC::JobHolderRow)
Q_DECLARE_METATYPE (LC::ProcessStateInfo)

Q_DECLARE_INTERFACE (IJobHolder, "org.Deviant.LeechCraft.IJobHolder/1.0")
