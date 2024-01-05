#pragma once
#include "../../pci.h"
#include "../../vfs.h"
#include "../../system.h"
#include "../../dma.h"
#include "../blockdev.h"
#include "defs.h" // include the struct definitions (yeey)

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
#define _mesa_assert(check, msg) do { if(!(check)) { printf(msg, #check); _mesa_unload(); } } while(0)

struct drm_nouveau_getparam {
	uint64_t param;
	uint64_t value;
};

struct drm_nouveau_channel_alloc {
	uint32_t   fb_ctxdma_handle;
	uint32_t   tt_ctxdma_handle;
	uint32_t   channel;
	uint32_t   pushbuf_domains;
	uint32_t   notifier_handle;
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

struct nouveau_cli {
	struct nvif_client base;
	struct nouveau_drm *drm;
	struct spinlock mutex;

	struct nvif_device device;
	struct nvif_mmu mmu;
	struct nouveau_vmm vmm;
	struct nouveau_vmm svm;
	struct nouveau_uvmm uvmm;

	struct nouveau_sched_entity sched_entity;
	const struct nvif_mclass *mem;

	struct list_head head;
	void *abi16;
	struct list_head objects;
	char name[32];

	struct work_struct work;
	struct list_head worker;
	struct spinlock lock;
};

struct nouveau_cli_work {
	void (*func)(struct nouveau_cli_work *);
	struct nouveau_cli *cli;
	struct list_head head;

	struct dma_fence *fence;
	struct dma_fence_cb cb;
};

static inline struct nouveau_uvmm * nouveau_cli_uvmm(struct nouveau_cli *cli)
{
	if (!cli || !cli->uvmm.vmm.cli)
		return NULL;

	return &cli->uvmm;
}

static inline struct nouveau_uvmm * nouveau_cli_uvmm_locked(struct nouveau_cli *cli)
{
	struct nouveau_uvmm *uvmm;

	mutex_lock(&cli->mutex);
	uvmm = nouveau_cli_uvmm(cli);
	mutex_unlock(&cli->mutex);

	return uvmm;
}

struct pcidev* mesa_check()
{
    for(int i = 0;i < 20;i++)
        if(pci_devs[i].pclass == 0x3000)
            return &pci_devs[i];
    return (struct pcidev*)NULL_PTR;
}

static int nouveau_ioctl_gem_pushbuf(int fd, unsigned long request, void *arg)
{
   struct drm_nouveau_gem_pushbuf *submit = arg;
   submit->vram_available = 3ULL << 30;
   submit->gart_available = 1ULL << 40;
   return 0;
}

struct drm_device* mesa_frompci(struct pcidev* dev)
{

}

int nouveau_gem_ioctl_new(struct drm_device *dev, void *data,
		      struct drm_file *file_priv)
{
	struct nouveau_cli *cli = nouveau_cli(file_priv);
	struct drm_nouveau_gem_new *req = data;
	struct nouveau_bo *nvbo = NULL_PTR;
	int ret = 0;

	/* If uvmm wasn't initialized until now disable it completely to prevent
	 * userspace from mixing up UAPIs.
	 */
	nouveau_cli_disable_uvmm_noinit(cli);

	ret = nouveau_gem_new(cli, req->info.size, req->align,
			      req->info.domain, req->info.tile_mode,
			      req->info.tile_flags, &nvbo);
	if (ret)
		return ret;

	ret = drm_gem_handle_create(file_priv, &nvbo->bo.base,
				    &req->info.handle);
	if (ret == 0) {
		ret = nouveau_gem_info(file_priv, &nvbo->bo.base, &req->info);
		if (ret)
			drm_gem_handle_delete(file_priv, req->info.handle);
	}

	/* drop reference from allocate - handle holds it now */
	drm_gem_object_put(&nvbo->bo.base);
	return ret;
}

void _mesa_unload()
{

}

void mesa_init()
{
    struct pcidev* dev = mesa_check(); // checks for card in pci dev list
    _mesa_assert(dev != NULL_PTR, "No device found.");
}