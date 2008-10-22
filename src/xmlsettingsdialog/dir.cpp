/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#include "dir.h"

Dir::Dir (QObject *parent)
: QObject (parent)
{
}

Dir::Dir (const Dir& obj)
: QObject (obj.parent ())
, QDir (obj)
{
}

Dir::~Dir ()
{
}

