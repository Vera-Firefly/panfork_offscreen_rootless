/*
 * Copyright (C) 2022 Icecream95
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Library for interfacing with kbase */
#ifndef PAN_BASE_H
#define PAN_BASE_H

#include "util/u_dynarray.h"

typedef uint64_t base_va;
struct base_ptr {
        void *cpu;
        base_va gpu;
};

struct kbase_syncobj;

struct kbase_cs {
        void *user_io;
};

struct kbase;
typedef struct kbase *kbase;

struct kbase {
        unsigned setup_state;

        int fd;
        unsigned page_size;
        unsigned cs_queue_count;

        unsigned gpuprops_size;
        void *gpuprops;

        void *tracking_region;
        void *csf_user_reg;
        base_va tiler_heap_va;
        base_va tiler_heap_header;

        bool needs_ext_res;
        uint8_t atom_number;

        uint8_t csg_handle;
        uint32_t csg_uid;
        unsigned num_csi;

        struct util_dynarray gem_handles;


        void (*close)(kbase k);

        bool (*get_pan_gpuprop)(kbase k, unsigned name, uint64_t *value);
        bool (*get_mali_gpuprop)(kbase k, unsigned name, uint64_t *value);

        struct base_ptr (*alloc)(kbase k, size_t size,
                                 unsigned pan_flags,
                                 unsigned mali_flags);
        void (*free)(kbase k, base_va va);

        void (*cache_clean)(void *ptr, size_t size);
        void (*cache_invalidate)(void *ptr, size_t size);

        /* <= v9 GPUs */
        bool (*submit)(kbase k, uint64_t va, unsigned req,
                       struct kbase_syncobj *o,
                       struct util_dynarray ext_res);

        /* >= v10 GPUs */
        // TODO: Pass in a priority?
        struct kbase_cs (*cs_bind)(kbase k, base_va va, unsigned size);
        void (*cs_term)(kbase k, struct kbase_cs *cs, base_va va);

        bool (*cs_submit)(kbase k, struct kbase_cs *cs, unsigned insert_offset,
                          struct kbase_syncobj *o);
        bool (*cs_wait)(kbase k, struct kbase_cs *cs, unsigned extract_offset);

        /* syncobj functions */
        struct kbase_syncobj (*syncobj_create)(kbase k);
        struct kbase_syncobj (*syncobj_free)(kbase k);
        struct kbase_syncobj (*syncobj_dup)(kbase k, struct kbase_syncobj *o);
        /* TODO: timeout? (and for cs_wait) */
        bool (*syncobj_wait)(kbase k, struct kbase_syncobj *o);

        struct base_ptr (*import)(kbase k, int fd, size_t *size);

        void (*ctr_open)(kbase k);
        void (*ctr_set_enabled)(kbase k, bool enable);
        void (*ctr_dump)(kbase k);
};

bool kbase_open(kbase k, int fd, unsigned cs_queue_count);

bool kbase_open_old(kbase k);
bool kbase_open_new(kbase k);
bool kbase_open_csf(kbase k);

int kbase_alloc_gem_handle(kbase k, int fd);
void kbase_free_gem_handle(kbase k, int handle);

#endif