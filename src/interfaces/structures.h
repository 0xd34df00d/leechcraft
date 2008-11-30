#ifndef STRUCTURES_H
#define STRUCTURES_H

class QMenu;

namespace LeechCraft
{
	/** @brief Describes parameters of a download job.
	 *
	 * Describes where and what should be saved.
	 *
	 * @sa LeechCraft::TaskParameter
	 */
	struct DownloadParams
	{
		/** @brief What the user wants to download - this could be
		 * anything from URL to Magnet hash to local torrent file.
		 */
		QByteArray Resource_;
		/** @brief Where it wants the data to be saved.
		 */
		QString Location_;
	};

	/** @brief Describes single task parameter.
	 */
	enum TaskParameter
	{
		/** Use default parameters.
		 */
		NoParameters = 0,
		/** Task should be started automatically after addition.
		 */
		Autostart = 1,
		/** Task should not be saved in history.
		 */
		DoNotSaveInHistory = 2,
		/** Task is fetched from the clipboard.
		 */
		FromClipboard = 4,
		/** Task is fetched from common job addition dialog.
		 */
		FromCommonDialog = 8,
		/** Task is automatically generated, for example, this is a
		 * request from another plugin.
		 */
		FromAutomatic = 16,
		/** User should not be notified about task finish.
		 */
		DoNotNotifyUser = 32,
		/** Task is used internally and would not be visible to the user
		 * at all.
		 */
		Internal = 64,
		/** Task should not be saved as it would have no meaning after
		 * next start.
		 */
		NotPersistent = 128
	};

	Q_DECLARE_FLAGS (TaskParameters, TaskParameter);
};

Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::TaskParameters);

#endif

