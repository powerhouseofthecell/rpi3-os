#include "kernel/vmiter.hh"
#include "common/stdlib.hh"
#include "kernel/pagetable.hh"

const pageentry_t vmiter::zero_pe = 0;

uint64_t vmiter::range_perm(size_t sz) const {
    uint64_t p = perm();
    size_t rsz = pageoffmask(level_) + 1;
    if ((p & PTE_P) != 0 && sz > rsz) {
        if (sz > ((int64_t) va() < 0 ? 0 : VA_LOWEND) - va()) {
            return 0;
        }
        vmiter it(*this);
        sz += va() & (rsz - 1);
        do {
            sz -= rsz;
            it.next_range();
            p &= it.perm();
            rsz = pageoffmask(it.level_) + 1;
        } while ((p & PTE_P) != 0 && sz > rsz);
    }
    if ((p & PTE_P) != 0) {
        return p;
    } else {
        return 0;
    }
}

void vmiter::down() {
    // uart_puts("L");
    // uart_puts(itoa(level_, 10));
    // uart_puts(":");
    // uart_puts(itoa(pageindex(va_, level_), 10));
    // uart_puts(" contains ");

    // uart_hex(*pep_ >>32);
    // uart_hex(*pep_);
    // uart_puts("\n");
    while (
        (level_ > 0 && (*pep_ & (PTE_P | PTE_PS)) == PTE_P && (*pep_ & PTE_PAGE) == PTE_PAGE) || 
        (level_ > 1 && (*pep_ & (PTE_P | PTE_PS)) == PTE_P && (*pep_ & PTE_PAGE) == 0)
    ) {
        perm_ &= *pep_ | ~(PTE_P | PTE_W | PTE_U);
        --level_;
        uintptr_t pa = *pep_ & PTE_PAMASK;
        pagetable* pt = reinterpret_cast<pagetable*>(pa);
        pep_ = &pt->entry[pageindex(va_, level_)];

        // uart_puts("L");
        // uart_puts(itoa(level_, 10));
        // uart_puts(":");
        // uart_puts(itoa(pageindex(va_, level_), 10));
        // uart_puts(" contains ");

        // uart_hex(*pep_ >>32);
        // uart_hex(*pep_);
        // uart_puts("\n");
    }

    if ((*pep_ & PTE_PAMASK) >= 0x100000000UL) {
        uart_puts("pagetable ");
        uart_hex((uint64_t)pt_>>32);
        uart_hex((uint64_t)pt_);
        uart_puts(" contains ");
        uart_hex(*pep_>>32);
        uart_hex(*pep_);

        assert(false);
        while (true);
        // panic("Page table %p may contain uninitialized memory!\n"
        //       "(Page table contents: %p)\n", pt_, *pep_);
    }
}

void vmiter::real_find(uintptr_t va) {
    if (level_ == MAX_PT_LEVEL || ((va_ ^ va) & ~pageoffmask(level_ + 1)) != 0) {
        level_ = MAX_PT_LEVEL;
        if (va_is_canonical(va)) {
            perm_ = initial_perm;
            pep_ = &pt_->entry[pageindex(va, level_)];
        } else {
            perm_ = 0;
            pep_ = const_cast<pageentry_t*>(&zero_pe);
        }
    } else {
        int curidx = (reinterpret_cast<uintptr_t>(pep_) & PAGEOFFMASK) >> 3;
        pep_ += pageindex(va, level_) - curidx;
    }
    va_ = va;
    down();
}

void vmiter::next() {
    int level = 0;
    if (level_ > 0 && !perm()) {
        level = level_;
    }
    real_find((va_ | pageoffmask(level)) + 1);
}

int vmiter::try_map(uintptr_t pa, int perm) {
    if (pa == (uintptr_t) -1 && perm == 0) {
        pa = 0;
    }
    assert(!(va_ & PAGEOFFMASK));
    if (perm & PTE_P) {
        assert(pa != (uintptr_t) -1);
        assert((pa & PTE_PAMASK) == pa);
    } else {
        assert(!(pa & PTE_P));
    }
    assert(!(perm & ~perm_ & (PTE_P | PTE_W | PTE_U)));

    // this goes down to the lowest level for respective mappings
    // page: L0
    // block: L1
    while (((level_ > 0 && (perm & PTE_PAGE) == PTE_PAGE)
        || (level_ > 1 && (perm & PTE_PAGE) == 0)) && perm
    ) {
        assert(!(*pep_ & PTE_P));
        pagetable* pt = (pagetable*) kalloc_page();
        if (!pt) {
            return -1;
        }
        memset(pt, 0x00, PAGESIZE);

        // uart_puts("setting L");
        // uart_puts(itoa(level_, 10));
        // uart_puts(" pt with perm ");
        // uart_hex(perm);
        // uart_puts(" @ ");
        // uart_hex((uint64_t) pt>>32);
        // uart_hex((uint64_t) pt);
        // uart_puts("\n");

        *pep_ = (uintptr_t) pt | perm | PTE_A;
        down();
    }

    if (level_ == 0 || (level_ == 1 && (perm & PTE_PAGE) == 0)) {
        // uart_puts("setting ");
        // uart_hex(perm);
        // uart_puts(" @ ");
        // uart_hex(pa>>32);
        // uart_hex(pa);
        // uart_puts("\n");
        *pep_ = pa | perm;
    }
    return 0;
}


void ptiter::go(uintptr_t va) {
    level_ = MAX_PT_LEVEL;
    pep_ = &pt_->entry[pageindex(va, level_)];
    va_ = va;
    down(false);
}

void ptiter::down(bool skip) {
    int stop_level = 1;
    while (true) {
        if ((*pep_ & (PTE_P | PTE_PS)) == PTE_P && !skip) {
            if (level_ == stop_level) {
                break;
            } else {
                --level_;
                uintptr_t pa = *pep_ & PTE_PAMASK;
                pagetable* pt = reinterpret_cast<pagetable*>(pa);
                pep_ = &pt->entry[pageindex(va_, level_)];
            }
        } else {
            uintptr_t va = (va_ | pageoffmask(level_)) + 1;
            if ((va ^ va_) & ~pageoffmask(level_ + 1)) {
                // up one level
                if (level_ == MAX_PT_LEVEL) {
                    va_ = VA_NONCANONMAX + 1;
                    return;
                }
                stop_level = level_ + 1;
                level_ = MAX_PT_LEVEL;
                pep_ = &pt_->entry[pageindex(va_, level_)];
            } else {
                ++pep_;
                va_ = va;
            }
            skip = false;
        }
    }
}
