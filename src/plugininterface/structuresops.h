#ifndef PLUGININTERFACE_STRUCTURESOPS_H
#define PLUGININTERFACE_STRUCTURESOPS_H
#include <QDataStream>
#include "../interfaces/structures.h"
#include "config.h"

PLUGININTERFACE_API QDataStream& operator<< (QDataStream& out, const LeechCraft::DownloadEntity& e);
PLUGININTERFACE_API QDataStream& operator>> (QDataStream& in, LeechCraft::DownloadEntity& e);

#endif

