#ifndef _I386_PGTABLE_2LEVEL_H
#define _I386_PGTABLE_2LEVEL_H

/*
 * traditional i386 two-level paging structure:
 */

// 表示在线性地址中，从低位（0，一般在右侧）开始
// 的index，是index=22，也就是第23位
//
#define PGDIR_SHIFT	22
//
// 这里定义了每个PGD中所能容纳的表项数量，为1024个
// 而32位系统中每个指针长度是4byte，所以一个PGD占据
// 的空间是1024 * 4byte = 4KB
//
#define PTRS_PER_PGD	1024

/*
 * the i386 is two-level, so we don't really have any
 * PMD directory physically.
 */

// 此文件中的各种宏定义都是关于两层映射的，所以自然PMD的
// 起始位置为22，说明关于PMD的偏移量是0。2 ** 0 = 1，
// 所以PTRS_PER_PMD是1。
//
#define PMD_SHIFT	22
#define PTRS_PER_PMD	1

// 这里定义了每个PT的容量，1024个表项
//
#define PTRS_PER_PTE	1024

#define pte_ERROR(e) \
	printk("%s:%d: bad pte %08lx.\n", __FILE__, __LINE__, (e).pte_low)
#define pmd_ERROR(e) \
	printk("%s:%d: bad pmd %08lx.\n", __FILE__, __LINE__, pmd_val(e))
#define pgd_ERROR(e) \
	printk("%s:%d: bad pgd %08lx.\n", __FILE__, __LINE__, pgd_val(e))

/*
 * The "pgd_xxx()" functions here are trivial for a folded two-level
 * setup: the pgd is never bad, and a pmd always exists (as it's folded
 * into the pgd entry)
 */
extern inline int pgd_none(pgd_t pgd)		{ return 0; }
extern inline int pgd_bad(pgd_t pgd)		{ return 0; }
extern inline int pgd_present(pgd_t pgd)	{ return 1; }
#define pgd_clear(xp)				do { } while (0)

/*
 * Certain architectures need to do special things when PTEs
 * within a page table are directly modified.  Thus, the following
 * hook is made available.
 */

// 将pteval的值填写入一个PTE中
//
#define set_pte(pteptr, pteval) (*(pteptr) = pteval)
/*
 * (pmds are folded into pgds so this doesnt get actually called,
 * but the define is needed for a generic inline function.)
 */
#define set_pmd(pmdptr, pmdval) (*(pmdptr) = pmdval)
#define set_pgd(pgdptr, pgdval) (*(pgdptr) = pgdval)

#define pgd_page(pgd) \
((unsigned long) __va(pgd_val(pgd) & PAGE_MASK))

extern inline pmd_t * pmd_offset(pgd_t * dir, unsigned long address)
{
	return (pmd_t *) dir;
}
#define ptep_get_and_clear(xp)	__pte(xchg(&(xp)->pte_low, 0))
#define pte_same(a, b)		((a).pte_low == (b).pte_low)
// 当P标志位为1，代表对应的物理页面在内存中时，pte高20位代表
// 物理页面的起始地址，这就够了，因为作为一个起始地址，低12位
// 一定是0
//
// 在mem_map中可以直接用高20位作为索引
//
// 这里的pte_page是在说：
// 1. 求x.pte_low >> 12, 等于pte的高20位；
// 2. 强转为unsigned long然后去mem_map中索引；
// 3. 返回一个page_t，就是对应的物理页面数据结构；

#define pte_page(x)		(mem_map+((unsigned long)(((x).pte_low >> PAGE_SHIFT))))
// 检验表项是否为空（尚未建立映射）
#define pte_none(x)		(!(x).pte_low)
// page_nr << 22 + pgprot = pte
#define __mk_pte(page_nr,pgprot) __pte(((page_nr) << PAGE_SHIFT) | pgprot_val(pgprot))

#endif /* _I386_PGTABLE_2LEVEL_H */
