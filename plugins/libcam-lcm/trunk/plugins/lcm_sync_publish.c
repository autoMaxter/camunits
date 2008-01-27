#include <stdio.h>
#include <stdlib.h>

#include <libcam/plugin.h>
#include <lcm/lcm.h>

#include "camlcm_sync_t.h"
#include "lcm_sync_publish.h"

#define err(args...) fprintf(stderr, args)

#define DEFAULT_LCM_URL "udpm://?transmit_only=true"

struct _CamlcmSyncPub {
    CamUnit parent;
    CamUnitControl *lcm_url_ctl;
    lcm_t * lcm;
};

struct _CamlcmSyncPubClass {
    CamUnitClass parent_class;
};

GType camlcm_syncpub_get_type (void);

static CamlcmSyncPub * camlcm_syncpub_new(void);
static void camlcm_syncpub_finalize (GObject *obj);
static void on_input_frame_ready (CamUnit * super, const CamFrameBuffer *inbuf,
        const CamUnitFormat *infmt);
static void on_input_format_changed (CamUnit *super, 
        const CamUnitFormat *infmt);
static gboolean _try_set_control (CamUnit *super, 
        const CamUnitControl *ctl, const GValue *proposed, GValue *actual);

CAM_PLUGIN_TYPE (CamlcmSyncPub, camlcm_syncpub, CAM_TYPE_UNIT);

/* These next two functions are required as entry points for the
 * plug-in API. */
void
cam_plugin_initialize (GTypeModule * module)
{
    camlcm_syncpub_register_type (module);
}

CamUnitDriver *
cam_plugin_create (GTypeModule * module)
{
    return cam_unit_driver_new_stock_full ("output", "lcm_syncpub",
            "LCM Sync Publish", 0, (CamUnitConstructor)camlcm_syncpub_new,
            module);
}

static void
camlcm_syncpub_init (CamlcmSyncPub *self)
{
    // constructor.  Initialize the unit with some reasonable defaults here.
    self->lcm = lcm_create (DEFAULT_LCM_URL);
    self->lcm_url_ctl = cam_unit_add_control_string (CAM_UNIT (self), 
        "lcm-url", "LCM URL", DEFAULT_LCM_URL, 1);
    g_signal_connect (G_OBJECT (self), "input-format-changed",
            G_CALLBACK (on_input_format_changed), self);
}

static void
camlcm_syncpub_class_init (CamlcmSyncPubClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = camlcm_syncpub_finalize;
    klass->parent_class.on_input_frame_ready = on_input_frame_ready;
    klass->parent_class.try_set_control = _try_set_control;
}

static void
camlcm_syncpub_finalize (GObject *obj)
{
    CamlcmSyncPub * self = CAMLCM_SYNCPUB (obj);
    if (self->lcm) lcm_destroy (self->lcm);
    G_OBJECT_CLASS (camlcm_syncpub_parent_class)->finalize(obj);
}

static CamlcmSyncPub * 
camlcm_syncpub_new()
{
    return CAMLCM_SYNCPUB(g_object_new(CAMLCM_TYPE_SYNCPUB, NULL));
}

static void
on_input_format_changed (CamUnit *super, const CamUnitFormat *infmt)
{
    cam_unit_remove_all_output_formats (super);
    if (! infmt) return;
    cam_unit_add_output_format_full (super, infmt->pixelformat,
            infmt->name, infmt->width, infmt->height, 
            infmt->row_stride, infmt->max_data_size);
}

static void 
on_input_frame_ready (CamUnit *super, const CamFrameBuffer *inbuf, 
        const CamUnitFormat *infmt)
{
    CamlcmSyncPub *self = CAMLCM_SYNCPUB(super);
    if (self->lcm) {
        camlcm_sync_t msg;
        msg.utime = inbuf->timestamp;
        camlcm_sync_t_publish (self->lcm, CAMLCM_SYNC_CHANNEL, &msg);
    }
    cam_unit_produce_frame (super, inbuf, infmt);
    return;
}


static gboolean 
_try_set_control (CamUnit *super, const CamUnitControl *ctl, 
        const GValue *proposed, GValue *actual)
{
    CamlcmSyncPub *self = CAMLCM_SYNCPUB (super);
    if (ctl == self->lcm_url_ctl) {
        const char *url = g_value_get_string (proposed);
        if (self->lcm) {
            lcm_destroy (self->lcm);
        }
        self->lcm = lcm_create (url);
    } 

    g_value_copy (proposed, actual);
    return TRUE;
}
