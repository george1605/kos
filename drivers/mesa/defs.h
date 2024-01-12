#pragma once
#include "../../irq.c"
#include "../../vfs.h"
#include "../../id.h" // id allocation
#define NOUVEAU_GETPARAM_PCI_VENDOR      3
#define NOUVEAU_GETPARAM_PCI_DEVICE      4
#define NOUVEAU_GETPARAM_BUS_TYPE        5
#define NOUVEAU_GETPARAM_FB_SIZE         8
#define NOUVEAU_GETPARAM_AGP_SIZE        9
#define NOUVEAU_GETPARAM_CHIPSET_ID      11
#define NOUVEAU_GETPARAM_VM_VRAM_BASE    12
#define NOUVEAU_GETPARAM_GRAPH_UNITS     13
#define NOUVEAU_GETPARAM_PTIMER_TIME     14
#define NOUVEAU_GETPARAM_HAS_BO_USAGE    15
#define NOUVEAU_GETPARAM_HAS_PAGEFLIP    16

#define NOUVEAU_GEM_PUSHBUF_SYNC  (1ULL << 0)
#define NOUVEAU_GEM_RELOC_LOW  (1 << 0)
#define NOUVEAU_GEM_RELOC_HIGH (1 << 1)
#define NOUVEAU_GEM_RELOC_OR   (1 << 2)
#define NOUVEAU_GEM_MAX_RELOCS 1024

enum switch_power_state {
	DRM_SWITCH_POWER_ON = 0,
	DRM_SWITCH_POWER_OFF = 1,
	DRM_SWITCH_POWER_CHANGING = 2,
	DRM_SWITCH_POWER_DYNAMIC_OFF = 3,
};

struct drm_nouveau_gem_pushbuf {
	uint32_t channel;
	uint32_t nr_buffers;
	uint64_t buffers;
	uint32_t nr_relocs;
	uint32_t nr_push;
	uint64_t relocs;
	uint64_t push;
	uint32_t suffix0;
	uint32_t suffix1;
	uint64_t vram_available;
	uint64_t gart_available;
};

struct drm_fb_helper_surface_size {
	uint32_t fb_width;
	uint32_t fb_height;
	uint32_t surface_width;
	uint32_t surface_height;
	uint32_t surface_bpp;
	uint32_t surface_depth;
};

enum drm_gem_object_status {
	DRM_GEM_OBJECT_RESIDENT  = BIT(0),
	DRM_GEM_OBJECT_PURGEABLE = BIT(1),
};

struct drm_nouveau_getparam {
	uint64_t param;
	uint64_t value;
};

struct drm_nouveau_channel_alloc {
	uint32_t fb_ctxdma_handle;
	uint32_t tt_ctxdma_handle;
	uint32_t channel;
	uint32_t pushbuf_domains;
	uint32_t notifier_handle;
	struct {
		uint32_t handle;
		uint32_t grclass;
	} subchan[8];
	uint32_t nr_subchan;
};

struct drm_nouveau_channel_free {
	uint32_t channel;
};

struct drm_nouveau_gem_info {
	uint32_t handle;
	uint32_t domain;
	uint64_t size;
	uint64_t offset;
	uint64_t map_handle;
	uint32_t tile_mode;
	uint32_t tile_flags;
};

struct drm_nouveau_gem_new {
	struct drm_nouveau_gem_info info;
	uint32_t channel_hint;
	uint32_t align;
};

struct drm_modeset_lock {
	struct ww_mutex mutex;
	struct list_head head;
};

struct drm_modeset_acquire_ctx {
	struct ww_acquire_ctx ww_ctx;
	struct drm_modeset_lock *contended;

	uint32_t stack_depot;
	struct list_head locked;
	char trylock_only;
	char interruptible;
};

struct drm_mode_config {
	struct spinlock mutex;
	struct drm_modeset_lock connection_mutex;
	struct drm_modeset_acquire_ctx *acquire_ctx;
	struct spinlock idr_mutex;
	struct idr object_idr;
	struct idr tile_idr;

	struct spinlock fb_lock;
	int num_fb;
	struct list_head fb_list;
	struct spinlock connector_list_lock;
	/**
	 * @num_connector: Number of connectors on this device. Protected by
	 * @connector_list_lock.
	 */
	int num_connector;
	/**
	 * @connector_ida: ID allocator for connector indices.
	 */
	struct ida connector_ida;
	struct list_head connector_list;
	struct llist_head connector_free_list;
	/**
	 * @connector_free_work: Work to clean up @connector_free_list.
	 */
	struct work_struct connector_free_work;

	/**
	 * @num_encoder:
	 *
	 * Number of encoders on this device. This is invariant over the
	 * lifetime of a device and hence doesn't need any locks.
	 */
	int num_encoder;
	/**
	 * @encoder_list:
	 *
	 * List of encoder objects linked with &drm_encoder.head. This is
	 * invariant over the lifetime of a device and hence doesn't need any
	 * locks.
	 */
	struct list_head encoder_list;

	/**
	 * @num_total_plane:
	 *
	 * Number of universal (i.e. with primary/curso) planes on this device.
	 * This is invariant over the lifetime of a device and hence doesn't
	 * need any locks.
	 */
	int num_total_plane;
	struct list_head plane_list;

	int num_crtc;
	struct list_head crtc_list;
	struct list_head property_list;
	struct list_head privobj_list;

	int min_width, min_height;
	int max_width, max_height;
	const struct drm_mode_config_funcs *funcs;
	size_t fb_base;

	char poll_enabled;
	char poll_running;
	char delayed_event;
	struct delayed_work output_poll_work;

	struct spinlock blob_lock;

	/**
	 * @property_blob_list:
	 *
	 * List of all the blob property objects linked with
	 * &drm_property_blob.head. Protected by @blob_lock.
	 */
	struct list_head property_blob_list;

	/* pointers to standard properties */

	/**
	 * @edid_property: Default connector property to hold the EDID of the
	 * currently connected sink, if any.
	 */
	struct drm_property *edid_property;
	/**
	 * @dpms_property: Default connector property to control the
	 * connector's DPMS state.
	 */
	struct drm_property *dpms_property;
	/**
	 * @path_property: Default connector property to hold the DP MST path
	 * for the port.
	 */
	struct drm_property *path_property;
	/**
	 * @tile_property: Default connector property to store the tile
	 * position of a tiled screen, for sinks which need to be driven with
	 * multiple CRTCs.
	 */
	struct drm_property *tile_property;
	/**
	 * @link_status_property: Default connector property for link status
	 * of a connector
	 */
	struct drm_property *link_status_property;
	/**
	 * @plane_type_property: Default plane property to differentiate
	 * CURSOR, PRIMARY and OVERLAY legacy uses of planes.
	 */
	struct drm_property *plane_type_property;
	/**
	 * @prop_src_x: Default atomic plane property for the plane source
	 * position in the connected &drm_framebuffer.
	 */
	struct drm_property *prop_src_x;
	/**
	 * @prop_src_y: Default atomic plane property for the plane source
	 * position in the connected &drm_framebuffer.
	 */
	struct drm_property *prop_src_y;
	struct drm_property *prop_src_w;
	struct drm_property *prop_src_h;
	struct drm_property *prop_crtc_x;
	struct drm_property *prop_crtc_y;
	struct drm_property *prop_crtc_w;
	struct drm_property *prop_crtc_h;
	struct drm_property *prop_fb_id;
	struct drm_property *prop_in_fence_fd;
	struct drm_property *prop_out_fence_ptr;

	struct drm_property *prop_crtc_id;
	struct drm_property *prop_fb_damage_clips;
	struct drm_property *prop_active;
	struct drm_property *prop_mode_id;
	struct drm_property *prop_vrr_enabled;
	struct drm_property *dvi_i_subconnector_property;
	
	struct drm_property *dvi_i_select_subconnector_property;
	struct drm_property *dp_subconnector_property;
	struct drm_property *tv_subconnector_property;
	struct drm_property *tv_select_subconnector_property;
	struct drm_property *tv_mode_property;
	struct drm_property *tv_left_margin_property;
	struct drm_property *tv_right_margin_property;
	struct drm_property *tv_top_margin_property;
	struct drm_property *tv_bottom_margin_property;
	
	struct drm_property *tv_brightness_property;
	struct drm_property *tv_contrast_property;
	struct drm_property *tv_flicker_reduction_property;
	struct drm_property *tv_overscan_property;
	struct drm_property *tv_saturation_property;
	struct drm_property *tv_hue_property;
	struct drm_property *scaling_mode_property;
	struct drm_property *aspect_ratio_property;
	struct drm_property *content_type_property;

	struct drm_property *degamma_lut_property;
	struct drm_property *degamma_lut_size_property;
	struct drm_property *ctm_property;
	struct drm_property *gamma_lut_property;
	struct drm_property *gamma_lut_size_property;
	struct drm_property *suggested_x_property;
	struct drm_property *suggested_y_property;

	struct drm_property *non_desktop_property;
	struct drm_property *panel_orientation_property;
	struct drm_property *writeback_fb_id_property;
	struct drm_property *writeback_pixel_formats_property;
	struct drm_property *writeback_out_fence_ptr_property;
	struct drm_property *hdr_output_metadata_property;
	struct drm_property *content_protection_property;
	struct drm_property *hdcp_content_type_property;

	uint32_t preferred_depth, prefer_shadow;

	char prefer_shadow_fbdev;
	char quirk_addfb_prefer_xbgr_30bpp;
	char quirk_addfb_prefer_host_byte_order;
	char async_page_flip;
	char fb_modifiers_not_supported;
	char normalize_zpos;

	struct drm_property *modifiers_property;
	uint32_t cursor_width, cursor_height;
	struct drm_atomic_state *suspend_state;

	const struct drm_mode_config_helper_funcs *helper_private;
};

struct drm_device {
	int if_version;
	size_t ref;
	struct device *dev;
	struct {
		struct list_head resources;
		void *final_kfree;
		struct spinlock lock;
	} managed;

	const struct drm_driver *driver;
	void *dev_private;
	struct drm_minor *primary;
	struct drm_minor *render;

	struct drm_minor *accel;
	char registered;
	struct drm_master *master;
	uint32_t driver_features;
	char unplugged;

	struct inode *anon_inode;
	char *unique;
	struct spinlock struct_mutex;

	struct spinlock master_mutex;
	struct atomic open_count;
	struct spinlock filelist_mutex;
	struct list_head filelist;
	struct list_head filelist_internal;
	struct spinlock clientlist_mutex;

	struct list_head clientlist;
	char vblank_disable_immediate;
	struct drm_vblank_crtc *vblank;

	struct spinlock vblank_time_lock;
	struct spinlock vbl_lock;
	uint32_t max_vblank_count;

	struct list_head vblank_event_list;
	struct spinlock event_lock;

	unsigned int num_crtcs;

	struct drm_mode_config mode_config;
	struct spinlock object_name_lock;
	struct idr object_name_idr;

	struct drm_vma_offset_manager *vma_offset_manager;
	struct drm_vram_mm *vram_mm;
	enum switch_power_state switch_power_state;
	struct drm_fb_helper *fb_helper;
	struct dentry *debugfs_root;
    char irq_enabled;
	int irq;
};

void drm_set_irq(struct drm_device* dev, int irq_no, void(*func)(struct regs*))
{
    if(dev->irq_enabled) 
        return; // already installed
    dev->irq = irq_no;
    dev->irq_enabled = 1;
    irq_install_handler(irq_no, func);
}