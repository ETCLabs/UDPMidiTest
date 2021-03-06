/*
    Virtual Piano Widget for Qt4
    Copyright (C) 2008-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PIANODEFS_H
#define PIANODEFS_H

#ifdef VPIANO_PLUGINS
    #if defined(VPIANO_PLUGIN)
        #define VPIANO_EXPORT Q_DECL_EXPORT
    #else
        #define VPIANO_EXPORT Q_DECL_IMPORT
    #endif
#else
    #define VPIANO_EXPORT
#endif

#endif // PIANODEFS_H
