/*
 * mip/store_malloc.c: malloc-backed storage allocation for checked builds
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Limited, 1990-1992.
 * SPDX-Licence-Identifier: Apache-2.0
 */

/*
 * RCS $Revision$
 * Checkin $Date$
 * Revising $Author$
 */

#ifdef __STDC__
#  include <stdlib.h>
#  include <string.h>
#else
#  include "stddef.h"                                   /* for size_t */
#  include "strings.h"
extern char *malloc();
extern free();
#endif
#include "globals.h"
#include "store.h"
#include "defs.h"
#include "mcdep.h"  /* usrdbg(xxx) */
#include "errors.h"

#if defined(__has_feature)
#  if !__has_feature(address_sanitizer) && !__has_feature(memory_sanitizer)
#    error "store_malloc.c requires AddressSanitizer or MemorySanitizer"
#  endif
#elif !defined(__SANITIZE_ADDRESS__)
#  error "store_malloc.c requires AddressSanitizer or MemorySanitizer"
#endif

void ClearToNull(void **a, int32 n) {
  while (--n >= 0) a[n] = NULL;
}

/*
 * Match the arena allocator's host-pointer alignment so checked builds
 * preserve the original object layout and slack bytes.
 */
#define RR (sizeof(char *) - 1)

static void init_allocated_region(char *p, int32 n, int fill)
{
#ifndef ALLOC_DONT_CLEAR_MEMORY
    memset(p, fill, (size_t)n);
#else
    (void)p;
    (void)n;
    (void)fill;
#endif
}

typedef enum {
    AK_Perm,
    AK_Glob,
    AK_Bind,
    AK_Syn
} AllocKind;

typedef struct AllocNode AllocNode;
struct AllocNode {
    AllocNode *next;
    VoidStar data;
    unsigned32 seq;
    int32 size;
};

struct Mark {
    struct Mark *prev;
    unsigned32 syn_seq;
    unsigned32 bind_seq;
    bool unmarked;
};

typedef struct FreeList {
    struct FreeList *next;
    ssize_t rest[1];
} FreeList;

static AllocNode *perm_chain;
static AllocNode *glob_chain;
static AllocNode *bind_chain;
static AllocNode *syn_chain;

static unsigned32 bind_seq;
static unsigned32 syn_seq;

static int32 perm_bytes;
static int32 glob_bytes;
static int32 bind_bytes;
static int32 syn_bytes;
static int32 bindallmax;
static int32 synallmax;

static struct Mark *marklist;
static struct Mark *freemarks;

static int32 stuse_total, stuse_waste;
static int32 stuse[SU_Other-SU_Data+1];
static int32 maxAEstore;
struct CurrentFnDetails currentfunction;
char *phasename;

static VoidStar raw_alloc(size_t n)
{
    VoidStar p = malloc(n);
    if (p != NULL)
        return p;
#ifdef TARGET_IS_ARM
    if (usrdbg(DBG_ANY))
        cc_fatalerr(misc_fatalerr_space2);
    else
#endif
        cc_fatalerr(misc_fatalerr_space3);
    return 0;
}

static struct Mark *new_mark(void)
{
    struct Mark *p;
    if ((p = freemarks) != NULL)
        freemarks = p->prev;
    else
        p = (struct Mark *)raw_alloc(sizeof(*p));
    return p;
}

static AllocNode *new_allocnode(void)
{
    return (AllocNode *)raw_alloc(sizeof(AllocNode));
}

static VoidStar alloc_with_kind(AllocKind kind, StoreUse use, int32 n, int fill)
{
    AllocNode *node;
    int32 rounded;
    size_t actual;
    int32 *livep;
    AllocNode **chainp;

    node = new_allocnode();
    rounded = (n + RR) & ~(int32)RR;
    actual = (rounded == 0) ? 1u : (size_t)rounded;

    switch (kind)
    {
    case AK_Perm:
        chainp = &perm_chain;
        livep = &perm_bytes;
        node->seq = 0;
        break;
    case AK_Glob:
        chainp = &glob_chain;
        livep = &glob_bytes;
        node->seq = 0;
        break;
    case AK_Bind:
        chainp = &bind_chain;
        livep = &bind_bytes;
        node->seq = ++bind_seq;
        break;
    default:
        chainp = &syn_chain;
        livep = &syn_bytes;
        node->seq = ++syn_seq;
        break;
    }

    node->data = raw_alloc(actual);
    node->size = rounded;
    node->next = *chainp;
    *chainp = node;

    stuse_total += rounded;
    *livep += rounded;

    if (kind == AK_Glob)
        stuse[(int)use] += rounded;
    else if (kind == AK_Bind && *livep > bindallmax)
        bindallmax = *livep;
    else if (kind == AK_Syn && *livep > synallmax)
        synallmax = *livep;

    init_allocated_region((char *)node->data, rounded, fill);
    return node->data;
}

static void release_chain(AllocNode **chainp, int32 *livep)
{
    while (*chainp != NULL)
    {
        AllocNode *node = *chainp;
        *chainp = node->next;
        *livep -= node->size;
        free(node->data);
        free(node);
    }
}

static void release_newer_blocks(AllocNode **chainp, int32 *livep, unsigned32 seq)
{
    while (*chainp != NULL && (*chainp)->seq > seq)
    {
        AllocNode *node = *chainp;
        *chainp = node->next;
        *livep -= node->size;
        free(node->data);
        free(node);
    }
}

static void release_marks(struct Mark **listp)
{
    while (*listp != NULL)
    {
        struct Mark *next = (*listp)->prev;
        free(*listp);
        *listp = next;
    }
}

static VoidStar release_discarded(VoidStar p, int32 cells)
{
    FreeList *node;
    VoidStar q;
    AllocNode **pp;

    if (p == NULL)
        return 0;

    node = (FreeList *)p;
    q = (VoidStar)node->next;

    for (pp = &syn_chain; *pp != NULL; pp = &(*pp)->next)
    {
        if ((*pp)->data == p)
        {
            AllocNode *dead = *pp;
            *pp = dead->next;
            syn_bytes -= dead->size;
            free(dead->data);
            free(dead);
            return q;
        }
    }

    for (pp = &bind_chain; *pp != NULL; pp = &(*pp)->next)
    {
        if ((*pp)->data == p)
        {
            AllocNode *dead = *pp;
            *pp = dead->next;
            bind_bytes -= dead->size;
            free(dead->data);
            free(dead);
            return q;
        }
    }

    if (cells == 2)
        syserr(syserr_discard2, p);
    else
        syserr(syserr_discard3, p);
    return q;
}

VoidStar xglobal_cons2(StoreUse t, IPtr a, IPtr b)
{
    IPtr *p = (IPtr *)GlobAlloc(t, sizeof(IPtr[2]));
    p[0] = a;
    p[1] = b;
    return (VoidStar)p;
}

VoidStar xglobal_list3(StoreUse t, IPtr a, IPtr b, IPtr c)
{
    IPtr *p = (IPtr *)GlobAlloc(t, sizeof(IPtr[3]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    return (VoidStar)p;
}

VoidStar xglobal_list4(StoreUse t, IPtr a, IPtr b, IPtr c, IPtr d)
{
    IPtr *p = (IPtr *)GlobAlloc(t, sizeof(IPtr[4]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    return (VoidStar)p;
}

VoidStar xglobal_list5(StoreUse t, IPtr a, IPtr b, IPtr c, IPtr d, IPtr e)
{
    IPtr *p = (IPtr *)GlobAlloc(t, sizeof(IPtr[5]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    p[4] = e;
    return (VoidStar)p;
}

VoidStar xglobal_list6(StoreUse t, IPtr a, IPtr b, IPtr c, IPtr d, IPtr e, IPtr f)
{
    IPtr *p = (IPtr *)GlobAlloc(t, sizeof(IPtr[6]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    p[4] = e;
    p[5] = f;
    return (VoidStar)p;
}

VoidStar PermAlloc(int32 n)
{
    return alloc_with_kind(AK_Perm, SU_Other, n, 0xbb);
}

VoidStar GlobAlloc(StoreUse t, int32 n)
{
    return alloc_with_kind(AK_Glob, t, n, 0xbb);
}

VoidStar BindAlloc(int32 n)
{
    n = (n + RR) & ~(int32)RR;
    if (n > SEGSIZE)
        syserr(syserr_overlarge_store1, (long)n);
    return alloc_with_kind(AK_Bind, SU_Other, n, 0xcc);
}

VoidStar SynAlloc(int32 n)
{
    n = (n + RR) & ~(int32)RR;
    if (n > SEGSIZE)
        syserr(syserr_overlarge_store2, (long)n);
    return alloc_with_kind(AK_Syn, SU_Other, n, 0xaa);
}

VoidStar discard2(VoidStar p)
{
    return release_discarded(p, 2);
}

VoidStar discard3(VoidStar p)
{
    return release_discarded(p, 3);
}

VoidStar xsyn_list2(IPtr a, IPtr b)
{
    IPtr *p = (IPtr *)SynAlloc(sizeof(IPtr[2]));
    p[0] = a;
    p[1] = b;
    return (VoidStar)p;
}

VoidStar xbinder_list2(IPtr a, IPtr b)
{
    IPtr *p = (IPtr *)BindAlloc(sizeof(IPtr[2]));
    p[0] = a;
    p[1] = b;
    return (VoidStar)p;
}

VoidStar xbinder_list3(IPtr a, IPtr b, IPtr c)
{
    IPtr *p = (IPtr *)BindAlloc(sizeof(IPtr[3]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    return (VoidStar)p;
}

VoidStar xsyn_list3(IPtr a, IPtr b, IPtr c)
{
    IPtr *p = (IPtr *)SynAlloc(sizeof(IPtr[3]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    return (VoidStar)p;
}

VoidStar xsyn_list4(IPtr a, IPtr b, IPtr c, IPtr d)
{
    IPtr *p = (IPtr *)SynAlloc(sizeof(IPtr[4]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    return (VoidStar)p;
}

VoidStar xsyn_list5(IPtr a, IPtr b, IPtr c, IPtr d, IPtr e)
{
    IPtr *p = (IPtr *)SynAlloc(sizeof(IPtr[5]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    p[4] = e;
    return (VoidStar)p;
}

VoidStar xsyn_list6(IPtr a, IPtr b, IPtr c, IPtr d, IPtr e, IPtr f)
{
    IPtr *p = (IPtr *)SynAlloc(sizeof(IPtr[6]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    p[4] = e;
    p[5] = f;
    return (VoidStar)p;
}

VoidStar xsyn_list7(IPtr a, IPtr b, IPtr c, IPtr d, IPtr e, IPtr f, IPtr g)
{
    IPtr *p = (IPtr *)SynAlloc(sizeof(IPtr[7]));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    p[4] = e;
    p[5] = f;
    p[6] = g;
    return (VoidStar)p;
}

Mark *alloc_mark(void)
{
    struct Mark *p = new_mark();
    p->prev = marklist;
    p->syn_seq = syn_seq;
    p->bind_seq = bind_seq;
    p->unmarked = false;
    marklist = p;
    return p;
}

void alloc_unmark(Mark *mark)
{
    struct Mark *p = marklist;
    for (; p != NULL && p != mark; p = p->prev)
        continue;
    if (p == NULL)
        syserr(syserr_alloc_unmark);

    if (mark == marklist)
    {
        for (;;)
        {
            p = marklist;
            release_newer_blocks(&syn_chain, &syn_bytes, p->syn_seq);
            release_newer_blocks(&bind_chain, &bind_bytes, p->bind_seq);
            marklist = p->prev;
            p->prev = freemarks;
            freemarks = p;
            if (marklist == NULL || !marklist->unmarked)
                break;
        }
    }
    else
        p->unmarked = true;
}

void drop_local_store(void)
{
    if (marklist == NULL)
        syserr("corrupt alloc_marklist");
    release_newer_blocks(&syn_chain, &syn_bytes, marklist->syn_seq);
}

void alloc_reinit(void)
{
    if (marklist == NULL)
        syserr("corrupt alloc_marklist");
    if (syn_chain != NULL && syn_chain->seq > marklist->syn_seq)
        syserr(syserr_alloc_reinit);
    release_newer_blocks(&bind_chain, &bind_bytes, marklist->bind_seq);
}

void alloc_noteAEstoreuse(void)
{
    int32 n = syn_bytes + bind_bytes;
    if (n > maxAEstore)
        maxAEstore = n;
}

void show_store_use(void)
{
#ifdef ENABLE_STORE
    cc_msg(
        "Total store use (excluding stdio buffers/stack) %ld bytes\n",
        (long)stuse_total);
    cc_msg("Global store use %ld/%ld + %ld bytes\n",
        (long)glob_bytes, (long)glob_bytes, 0L);
    cc_msg(
        "  thereof %ld+%ld bytes pended relocation, %ld bytes pended data\n",
        (long)stuse[(int)SU_Xref],
        (long)stuse[(int)SU_Xsym],
        (long)stuse[(int)SU_Data]);
    cc_msg(
        "  %ld bytes symbols, %ld bytes top-level vars, %ld bytes types\n",
        (long)stuse[(int)SU_Sym],
        (long)stuse[(int)SU_Bind],
        (long)stuse[(int)SU_Type]);
    cc_msg(
        "  %ld bytes constants, %ld bytes pre-processor, %ld bytes wasted\n",
        (long)stuse[(int)SU_Const], (long)stuse[(int)SU_PP], (long)stuse_waste);
    cc_msg("Local store use %ld+%ld/%ld bytes - front end max %ld\n",
        (long)synallmax, (long)bindallmax,
        (long)(syn_bytes + bind_bytes),
        (long)maxAEstore);
#endif
}

void alloc_perfileinit(void)
{
    release_chain(&glob_chain, &glob_bytes);
    release_chain(&bind_chain, &bind_bytes);
    release_chain(&syn_chain, &syn_bytes);
    release_marks(&marklist);
    release_marks(&freemarks);

    stuse_total = 0;
    stuse_waste = 0;
    memclr(stuse, sizeof(stuse));
    bind_seq = 0;
    syn_seq = 0;
    bindallmax = 0;
    synallmax = 0;
    maxAEstore = 0;
    (void)alloc_mark();
}

void alloc_perfilefinalise(void)
{
    if (marklist == NULL || marklist->prev != NULL)
        syserr("corrupt alloc_marklist");
    drop_local_store();
    release_newer_blocks(&bind_chain, &bind_bytes, marklist->bind_seq);
    release_chain(&glob_chain, &glob_bytes);
    release_marks(&marklist);
    release_marks(&freemarks);
}

void alloc_initialise(void)
{
    perm_chain = NULL;
    glob_chain = NULL;
    bind_chain = NULL;
    syn_chain = NULL;
    marklist = NULL;
    freemarks = NULL;
    perm_bytes = 0;
    glob_bytes = 0;
    bind_bytes = 0;
    syn_bytes = 0;
    bind_seq = 0;
    syn_seq = 0;
}

void alloc_finalise(void)
{
    release_chain(&glob_chain, &glob_bytes);
    release_chain(&bind_chain, &bind_bytes);
    release_chain(&syn_chain, &syn_bytes);
    release_chain(&perm_chain, &perm_bytes);
    release_marks(&marklist);
    release_marks(&freemarks);
}
