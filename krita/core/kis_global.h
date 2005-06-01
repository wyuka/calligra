/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KISGLOBAL_H_
#define KISGLOBAL_H_

#include <config.h>

#include LCMS_HEADER
#include <limits.h>
#include <qglobal.h>
#include <kglobal.h>
#include <koGlobal.h>
#include <koUnit.h>

#include "config.h"


#define KRITA_VERSION VERSION

#define DBG_AREA_TILES 41000
#define DBG_AREA_CORE 41001
#define DBG_AREA_REGISTRY 40002
#define DBG_AREA_TOOLS 41003
#define DBG_AREA_CMS 41004
#define DBG_AREA_FILTERS 41005
#define DBG_AREA_PLUGINS 41006
#define DBG_AREA_FILE 41008

/**
 * Mime type for this app - not same as file type, but file types
 * can be associated with a mime type and are opened with applications
 * associated with the same mime type
 */
#define APP_MIMETYPE "application/x-krita"

/**
 * Mime type for native file format
 */
#define NATIVE_MIMETYPE "application/x-kra"

/**
 * Size of a quantum -- this could be 8, 16, 32 or 64 -- but for now, only 8 is possible.
 */
#ifndef QUANTUM_DEPTH
#define QUANTUM_DEPTH 8 // bits, i.e., one byte per channel
#endif

#if (QUANTUM_DEPTH == 8)
typedef Q_UINT8 QUANTUM;
const QUANTUM QUANTUM_MAX = UCHAR_MAX;
const QUANTUM OPACITY_TRANSPARENT = 0;
const QUANTUM OPACITY_OPAQUE = QUANTUM_MAX;
const QUANTUM MAX_SELECTED = QUANTUM_MAX;
const QUANTUM MIN_SELECTED = 0;
const QUANTUM SELECTION_THRESHOLD = 1;
#endif

const Q_INT32 IMG_DEFAULT_DEPTH = 4;
const Q_INT32 RENDER_HEIGHT = 128;
const Q_INT32 RENDER_WIDTH = 128;

enum enumInputDevice {
	INPUT_DEVICE_UNKNOWN,
	INPUT_DEVICE_MOUSE,	// Standard mouse
	INPUT_DEVICE_STYLUS,	// Wacom stylus
	INPUT_DEVICE_ERASER,	// Wacom eraser
	INPUT_DEVICE_PUCK	// Wacom puck
};

enum enumCursorStyle {
	CURSOR_STYLE_TOOLICON,
	CURSOR_STYLE_CROSSHAIR,
	CURSOR_STYLE_POINTER,
	CURSOR_STYLE_OUTLINE
};


enum enumDockerStyle {
	DOCKER_SLIDER,
	DOCKER_DOCKER,
	DOCKER_TOOLBOX
};


enum enumRotationDirection {
	CLOCKWISE,
	COUNTERCLOCKWISE,
};

/*
 * Most wacom pads have 512 levels of pressure; Qt only supports 256, and even
 * this is downscaled to 127 levels because the line would be too jittery, and
 * the amount of masks take too much memory otherwise.
 */
const Q_INT32 PRESSURE_LEVELS= 127;
const double PRESSURE_MIN = 0.0;
const double PRESSURE_MAX = 1.0;
const double PRESSURE_DEFAULT = (PRESSURE_MAX - PRESSURE_MIN) / 2;
const double PRESSURE_THRESHOLD = 5.0 / 255.0;

enum enumPaintStyles {
	PAINTSTYLE_HARD,
	PAINTSTYLE_SOFT
};

// If QUANTUM changes size, this should change, too.
typedef Q_UINT8 PIXELTYPE;


#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((QUANTUM) (257UL*(value)))

#endif // KISGLOBAL_H_

