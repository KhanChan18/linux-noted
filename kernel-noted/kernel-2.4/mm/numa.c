/*
 * Written by Kanoj Sarcar, SGI, Aug 1999
 */
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <linux/spinlock.h>

int numnodes = 1;	/* Initialized for UMA platforms */

static bootmem_data_t contig_bootmem_data;
pg_data_t contig_page_data = { bdata: &contig_bootmem_data };

#ifndef CONFIG_DISCONTIGMEM

/*
 * This is meant to be invoked by platforms whose physical memory starts
 * at a considerably higher value than 0. Examples are Super-H, ARM, m68k.
 * Should be invoked with paramters (0, 0, unsigned long *[], start_paddr).
 */
void __init free_area_init_node(int nid, pg_data_t *pgdat, struct page *pmap,
	unsigned long *zones_size, unsigned long zone_start_paddr, 
	unsigned long *zholes_size)
{
	free_area_init_core(0, &contig_page_data, &mem_map, zones_size, 
				zone_start_paddr, zholes_size, pmap);
}

#endif /* !CONFIG_DISCONTIGMEM */

struct page * alloc_pages_node(int nid, int gfp_mask, unsigned long order)
{
#ifdef CONFIG_NUMA
	return __alloc_pages(NODE_DATA(nid)->node_zonelists + gfp_mask, order);
#else
	return alloc_pages(gfp_mask, order);
#endif
}

#ifdef CONFIG_DISCONTIGMEM

#define LONG_ALIGN(x) (((x)+(sizeof(long))-1)&~((sizeof(long))-1))

static spinlock_t node_lock = SPIN_LOCK_UNLOCKED;

void show_free_areas_node(pg_data_t *pgdat)
{
	unsigned long flags;

	spin_lock_irqsave(&node_lock, flags);
	show_free_areas_core(pgdat);
	spin_unlock_irqrestore(&node_lock, flags);
}

/*
 * Nodes can be initialized parallely, in no particular order.
 */
void __init free_area_init_node(int nid, pg_data_t *pgdat, struct page *pmap,
	unsigned long *zones_size, unsigned long zone_start_paddr, 
	unsigned long *zholes_size)
{
	int i, size = 0;
	struct page *discard;

	if (mem_map == (mem_map_t *)NULL)
		mem_map = (mem_map_t *)PAGE_OFFSET;

	free_area_init_core(nid, pgdat, &discard, zones_size, zone_start_paddr,
					zholes_size, pmap);
	pgdat->node_id = nid;

	/*
	 * Get space for the valid bitmap.
	 */
	for (i = 0; i < MAX_NR_ZONES; i++)
		size += zones_size[i];
	size = LONG_ALIGN((size + 7) >> 3);
	pgdat->valid_addr_bitmap = (unsigned long *)alloc_bootmem_node(pgdat, size);
	memset(pgdat->valid_addr_bitmap, 0, size);
}

static struct page * alloc_pages_pgdat(pg_data_t *pgdat, int gfp_mask,
	unsigned long order)
{
    // gfp_mask参数其实要作为node_zonelists这个数组的下标
	return __alloc_pages(pgdat->node_zonelists + gfp_mask, order);
}

/*
 * This can be refined. Currently, tries to do round robin, instead
 * should do concentratic circle search, starting from current node.
 */

// gfp_mask参数表示的是分配策略；
// order表示的是要分配的物理块的大小，为2 ** order个页面构成的块；
//
struct page * alloc_pages(int gfp_mask, unsigned long order)
{
	struct page *ret = 0;
	pg_data_t *start, *temp;
#ifndef CONFIG_NUMA
	unsigned long flags;
	static pg_data_t *next = 0;
#endif
    // 这里表示参数order的上限是MAX_ORDER
	if (order >= MAX_ORDER)
		return NULL;
    //
    // 如果开启了CONFIG_NUMA编译选项，则要
    // 先获取CPU所在节点的pg_data_t数据结构队列
    //
#ifdef CONFIG_NUMA
	temp = NODE_DATA(numa_node_id());
#else
	spin_lock_irqsave(&node_lock, flags);
    // 这是UMA结构中的pg_data_t队列
	if (!next) next = pgdat_list;
	temp = next;
	next = next->node_next;
	spin_unlock_irqrestore(&node_lock, flags);
#endif
    // 从队列中间某个点（temp处）开始向后扫描，
    // 到了队尾再从前向temp扫描，直到找到一个
    // pg_data_t结构，然后返回它
    // 
	start = temp;
	while (temp) {
		if ((ret = alloc_pages_pgdat(temp, gfp_mask, order)))
			return(ret);
		temp = temp->node_next;
	}
	temp = pgdat_list;
	while (temp != start) {
		if ((ret = alloc_pages_pgdat(temp, gfp_mask, order)))
			return(ret);
		temp = temp->node_next;
	}
    // 如果完全没有找到则返回0
	return(0);
}

#endif /* CONFIG_DISCONTIGMEM */