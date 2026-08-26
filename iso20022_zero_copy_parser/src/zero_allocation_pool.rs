//! Bump-allocated contiguous arena memory pool for lock-free, zero-heap parsing.

use std::cell::UnsafeCell;

pub struct MemoryArena<const CAPACITY: usize> {
    storage: UnsafeCell<[u8; CAPACITY]>,
    offset: UnsafeCell<usize>,
}

unsafe impl<const CAPACITY: usize> Sync for MemoryArena<CAPACITY> {}

impl<const CAPACITY: usize> MemoryArena<CAPACITY> {
    pub const fn new() -> Self {
        Self {
            storage: UnsafeCell::new([0u8; CAPACITY]),
            offset: UnsafeCell::new(0),
        }
    }

    #[inline(always)]
    pub fn alloc_slice<'a>(&'a self, len: usize) -> Option<&'a mut [u8]> {
        unsafe {
            let current_offset = *self.offset.get();
            let aligned_offset = (current_offset + 7) & !7; // 8-byte alignment

            if aligned_offset + len > CAPACITY {
                return None;
            }

            *self.offset.get() = aligned_offset + len;
            let ptr = (*self.storage.get()).as_mut_ptr().add(aligned_offset);
            Some(std::slice::from_raw_parts_mut(ptr, len))
        }
    }

    #[inline(always)]
    pub fn reset(&self) {
        unsafe {
            *self.offset.get() = 0;
        }
    }

    #[inline(always)]
    pub fn allocated_bytes(&self) -> usize {
        unsafe { *self.offset.get() }
    }
}
