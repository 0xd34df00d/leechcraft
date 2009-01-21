#ifndef ICUSTOMPROVIDER_H
#define ICUSTOMPROVIDER_H
#include <QString>

/** @brief Interface for plugins providing custom facilities.
 *
 * This interface should be used by plugins which provide custom
 * abilities not related to LeechCraft and not accounted by other
 * interfaces. All communication goes via signal/slot connections.
 */
class ICustomProvider
{
public:
	/** @brief Queries the plugin whether it implements a given feature.
	 *
	 * @param[in] feature Queried feature.
	 * @return Query result.
	 */
	virtual bool ImplementsFeature (const QString& feature) const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~ICustomProvider () {}
};

Q_DECLARE_INTERFACE (ICustomProvider, "org.Deviant.LeechCraft.ICustomProvider/1.0");

#endif

