#ifndef __cam_unit_format_h__
#define __cam_unit_format_h__

#include "pixels.h"
#include <glib-object.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SECTION:unit_format
 * @short_description: Describes how to interpret the data in a #CamFrameBuffer
 * object.
 *
 * The CamUnitFormat class specifies how to interpret the data field in a
 * #CamFrameBuffer generated by a #CamUnit.  The most important is the
 * %pixelformat field, which specifies how the image is actually represented.
 * Typical values for %pixelformat are CAM_PIXEL_FORMAT_GRAY,
 * CAM_PIXEL_FORMAT_RGB, etc.  Additionally, the
 * CamUnitFormat class specifies the width and height of an image, the row
 * stride (for pixel formats where stride is meaningful), and the maximum
 * length of the image data (for variable length formats such as JPEG and other
 * compressed formats)
 *
 * Each instance of a #CamUnit provides a list of output formats that it is
 * able to produce.  On initialization, one of these output formats must be
 * specified.  Once a unit is initialized, it is expected to produce
 * #CamFrameBuffer objects (via the "frame-ready" signal) formatted according
 * to the specified CamUnitFormat.
 */

typedef struct _CamUnitFormat CamUnitFormat;
typedef struct _CamUnitFormatClass CamUnitFormatClass;

#define CAM_TYPE_UNIT_FORMAT  cam_unit_format_get_type()
#define CAM_UNIT_FORMAT(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        CAM_TYPE_UNIT_FORMAT, CamUnitFormat))
#define CAM_UNIT_FORMAT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
            CAM_TYPE_UNIT_FORMAT, CamUnitFormatClass))
#define CAM_IS_UNIT_FORMAT(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
            CAM_TYPE_UNIT_FORMAT))
#define CAM_IS_UNIT_FORMAT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE (\
            (klass), CAM_TYPE_UNIT_FORMAT))
#define CAM_UNIT_FORMAT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), \
            CAM_TYPE_UNIT_FORMAT, CamUnitFormatClass))

/**
 * CamUnitFormat:
 * @pixelformat: the #CamPixelFormat
 * @name: a human-understandable description of the format 
 * @row_stride: number of bytes separating rows.  For packed data, this is
 *              typically derived from pixelformat and width.  usually
 *              something like 
 *        <screen>stride = (width * cam_pixel_format_bpppixelformat) / 8</screen>. 
 *        Should be 0 for formats where stride is not meaningful (e.g.
 *        compressed formats or planar data).
 * @width: the width, in pixels, of the frame
 * @height: the height, in pixels, of the frame.
 */
struct _CamUnitFormat {
    GObject parent;

    /*< public >*/
    CamPixelFormat pixelformat;
    char * name;
    int row_stride;
    int width;
    int height;

    /*< protected >*/

    /*< private >*/
};

struct _CamUnitFormatClass {
    GObjectClass parent_class;
};

GType cam_unit_format_get_type (void);

/**
 * cam_unit_format_new:
 * @pfmt: the pixel format to use
 * @name: a human-readable description of the overall unit format.  If set to
 *        NULL, a default name will be automatically chosen.
 * @width:  image width, in pixels
 * @height: image height, in pixels
 * @row_stride: number of bytes separating rows.  Should be 0 when the term row
 *              stride is not meaningful for the specified pixel format.
 *
 * Returns: a newly created #CamUnitFormat
 */
CamUnitFormat *cam_unit_format_new (CamPixelFormat pfmt, const char *name, 
        int width, int height, int row_stride);

/**
 * cam_unit_format_equals:
 *
 * Returns: 1 if the two formats are equal in all values, 0 if not
 */
int cam_unit_format_equals(const CamUnitFormat *self, const CamUnitFormat *a);

#ifdef __cplusplus
}
#endif

#endif
