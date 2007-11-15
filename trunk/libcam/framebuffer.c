#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#define MALLOC_ALIGNED(s) malloc(s)
#else
#include <malloc.h>
#define MALLOC_ALIGNED(s) memalign(16,s)
#endif

#include "framebuffer.h"
//#include "dbg.h"

static void cam_framebuffer_finalize (GObject *obj);

G_DEFINE_TYPE (CamFrameBuffer, cam_framebuffer, G_TYPE_OBJECT);

static void
cam_framebuffer_init (CamFrameBuffer *self)
{
    self->data = NULL;
    self->length = 0;
    self->bytesused = 0;
    self->timestamp = 0;
    self->source_uid = 0;
    self->owns_data = 0;

    self->metadata = g_hash_table_new_full (g_str_hash, g_str_equal,
            free, free);
}

static void
cam_framebuffer_class_init (CamFrameBufferClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = cam_framebuffer_finalize;
}

static void
cam_framebuffer_finalize (GObject *obj)
{
    CamFrameBuffer *self = CAM_FRAMEBUFFER (obj);

    if (self->data && self->owns_data) {
        free (self->data);
    }
    self->data = NULL;
    self->length = 0;
    g_hash_table_destroy (self->metadata);

    G_OBJECT_CLASS (cam_framebuffer_parent_class)->finalize(obj);
}

CamFrameBuffer *
cam_framebuffer_new (uint8_t * data, int length)
{
    CamFrameBuffer *self = 
        CAM_FRAMEBUFFER (g_object_new (CAM_TYPE_FRAMEBUFFER, NULL));

    self->data = data;
    self->length = length;
    self->owns_data = 0;
    return self;
}

CamFrameBuffer *
cam_framebuffer_new_alloc (int length)
{
    CamFrameBuffer *self = 
        CAM_FRAMEBUFFER (g_object_new (CAM_TYPE_FRAMEBUFFER, NULL));
    self->data = (uint8_t*) MALLOC_ALIGNED (length);
    self->length = length;
    self->owns_data = 1;
    return self;
}

static void
_copy_keyval (void *key, void *value, void *user_data)
{
    GHashTable *dest_ht = user_data;
    g_hash_table_replace (dest_ht, strdup (key), strdup (value));
}

void
cam_framebuffer_copy_metadata (CamFrameBuffer * self, 
        const CamFrameBuffer *from)
{
    self->timestamp = from->timestamp;
    self->source_uid = from->source_uid;
    //g_hash_table_remove_all (self->metadata);
    g_hash_table_foreach (from->metadata, _copy_keyval, self->metadata);
}

gboolean 
cam_framebuffer_metadata_get (const CamFrameBuffer *self,
        const char *key, char **val, int *len)
{
    char *_val = g_hash_table_lookup (self->metadata, key);
    if (!_val) {
        if (val) *val = NULL;
        if (len) *len = 0;
        return FALSE;
    }
    if (len) *len = strlen (_val);
    if (val) *val = strdup (_val);
    return TRUE;
}

void 
cam_framebuffer_metadata_set (CamFrameBuffer *self, const char *key,
        const char *value)
{
    if (!value) {
        g_warning ("refusing to set NULL value in metadata dictionary");
        return;
    }
    if (!key) {
        g_warning ("refusing to set NULL key in metadata dictionary");
        return;
    }
    g_hash_table_replace (self->metadata, strdup (key), strdup (value));
}