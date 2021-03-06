/*
  Copyright 2012-2019 David Robillard <http://drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef PUGL_DEMO_UTILS_H
#define PUGL_DEMO_UTILS_H

#include "pugl/pugl.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	double lastReportTime;
} PuglFpsPrinter;

typedef float vec4[4];
typedef vec4  mat4[4];

// clang-format off

static const float cubeStripVertices[] = {
	-1.0f,  1.0f,  1.0f, // Front top left
	 1.0f,  1.0f,  1.0f, // Front top right
	-1.0f, -1.0f,  1.0f, // Front bottom left
	 1.0f, -1.0f,  1.0f, // Front bottom right
	 1.0f, -1.0f, -1.0f, // Back bottom right
	 1.0f,  1.0f,  1.0f, // Front top right
	 1.0f,  1.0f, -1.0f, // Back top right
	-1.0f,  1.0f,  1.0f, // Front top left
	-1.0f,  1.0f, -1.0f, // Back top left
	-1.0f, -1.0f,  1.0f, // Front bottom left
	-1.0f, -1.0f, -1.0f, // Back bottom left
	 1.0f, -1.0f, -1.0f, // Back bottom right
	-1.0f,  1.0f, -1.0f, // Back top left
	 1.0f,  1.0f, -1.0f  // Back top right
};

static const float cubeFrontLineLoop[] = {
	-1.0f,  1.0f,  1.0f, // Front top left
	 1.0f,  1.0f,  1.0f, // Front top right
	 1.0f, -1.0f,  1.0f, // Front bottom right
	-1.0f, -1.0f,  1.0f, // Front bottom left
};

static const float cubeBackLineLoop[] = {
	-1.0f,  1.0f, -1.0f, // Back top left
	 1.0f,  1.0f, -1.0f, // Back top right
	 1.0f, -1.0f, -1.0f, // Back bottom right
	-1.0f, -1.0f, -1.0f, // Back bottom left
};

static const float cubeSideLines[] = {
	-1.0f,  1.0f,  1.0f, // Front top left
	-1.0f,  1.0f, -1.0f, // Back top left

	-1.0f, -1.0f,  1.0f, // Front bottom left
	-1.0f, -1.0f, -1.0f, // Back bottom left

	 1.0f,  1.0f,  1.0f, // Front top right
	 1.0f,  1.0f, -1.0f, // Back top right

	 1.0f, -1.0f,  1.0f, // Front bottom right
	 1.0f, -1.0f, -1.0f, // Back bottom right
};

// clang-format on

static inline void
mat4Identity(mat4 m)
{
	for (int c = 0; c < 4; ++c) {
		for (int r = 0; r < 4; ++r) {
			m[c][r] = c == r ? 1.0f : 0.0f;
		}
	}
}

static inline void
mat4Translate(mat4 m, const float x, const float y, const float z)
{
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

static inline void
mat4Mul(mat4 m, mat4 a, mat4 b)
{
	for (int c = 0; c < 4; ++c) {
		for (int r = 0; r < 4; ++r) {
			m[c][r] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				m[c][r] += a[k][r] * b[c][k];
			}
		}
	}
}

static inline void
mat4Ortho(mat4        m,
          const float l,
          const float r,
          const float b,
          const float t,
          const float n,
          const float f)
{
	m[0][0] = 2.0f / (r - l);
	m[0][1] = m[0][2] = m[0][3] = 0.0f;

	m[1][1] = 2.0f / (t - b);
	m[1][0] = m[1][2] = m[1][3] = 0.0f;

	m[2][2] = -2.0f / (f - n);
	m[2][0] = m[2][1] = m[2][3] = 0.0f;

	m[3][0] = -(r + l) / (r - l);
	m[3][1] = -(t + b) / (t - b);
	m[3][2] = -(f + n) / (f - n);
	m[3][3] = 1.0f;
}

/** Calculate a projection matrix for a given perspective. */
static inline void
perspective(float* m, float fov, float aspect, float zNear, float zFar)
{
	const float h     = tanf(fov);
	const float w     = h / aspect;
	const float depth = zNear - zFar;
	const float q     = (zFar + zNear) / depth;
	const float qn    = 2 * zFar * zNear / depth;

	// clang-format off
	m[0]  = w;  m[1]  = 0;  m[2]  = 0;   m[3]  = 0;
	m[4]  = 0;  m[5]  = h;  m[6]  = 0;   m[7]  = 0;
	m[8]  = 0;  m[9]  = 0;  m[10] = q;   m[11] = -1;
	m[12] = 0;  m[13] = 0;  m[14] = qn;  m[15] = 0;
	// clang-format on
}

static inline void
puglPrintFps(const PuglWorld* world,
             PuglFpsPrinter*  printer,
             unsigned* const  framesDrawn)
{
	const double thisTime = puglGetTime(world);
	if (thisTime > printer->lastReportTime + 5) {
		const double fps = *framesDrawn / (thisTime - printer->lastReportTime);
		fprintf(stderr,
		        "%u frames in %.0f seconds = %.3f FPS\n",
		        *framesDrawn,
		        thisTime - printer->lastReportTime,
		        fps);

		printer->lastReportTime = thisTime;
		*framesDrawn            = 0;
	}
}

#endif // PUGL_DEMO_UTILS_H
