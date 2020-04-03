/*
 * Copyright (C) 2013 - 2015  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _LIBFM_QT_GLOBALS_
#define _LIBFM_QT_GLOBALS_

/* #include "fm-qt_export.h" */

#ifdef FM_QT_STATIC_DEFINE
#  define LIBFM_QT_API
#  define FM_QT_NO_EXPORT
#else
#  ifndef LIBFM_QT_API
#    ifdef fm_qt_EXPORTS
        /* We are building this library */
#      define LIBFM_QT_API __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define LIBFM_QT_API __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef FM_QT_NO_EXPORT
#    define FM_QT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef FM_QT_DEPRECATED
#  define FM_QT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef FM_QT_DEPRECATED_EXPORT
#  define FM_QT_DEPRECATED_EXPORT LIBFM_QT_API FM_QT_DEPRECATED
#endif

#ifndef FM_QT_DEPRECATED_NO_EXPORT
#  define FM_QT_DEPRECATED_NO_EXPORT FM_QT_NO_EXPORT FM_QT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FM_QT_NO_DEPRECATED
#    define FM_QT_NO_DEPRECATED
#  endif
#endif

#endif
