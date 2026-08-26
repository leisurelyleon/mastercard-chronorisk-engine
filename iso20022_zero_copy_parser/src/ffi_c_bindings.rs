//! C-ABI Foreign Function Interface (FFI) exports for direct C++ core interop.

use crate::message_validator::{MessageValidator, ParsedIsoTransaction};
use crate::zero_allocation_pool::MemoryArena;

static GLOBAL_ARENA: MemoryArena<{ 1024 * 1024 * 16 }> = MemoryArena::new(); // 16MB thread-safe arena

#[no_mangle]
pub unsafe extern "C" fn parse_iso_message_fast(
    xml_ptr: *const u8,
    xml_len: usize,
    out_transaction: *mut ParsedIsoTransaction,
) -> i32 {
    if xml_ptr.is_null() || out_transaction.is_null() || xml_len == 0 {
        return -1;
    }

    let payload = std::slice::from_raw_parts(xml_ptr, xml_len);
    let parsed = MessageValidator::parse_pacs008(payload);

    *out_transaction = parsed;

    if parsed.is_valid { 0 } else { 1 }
}

#[no_mangle]
pub extern "C" fn reset_parser_memory_arena() {
    GLOBAL_ARENA.reset();
}

#[no_mangle]
pub extern "C" fn get_parser_allocated_memory() -> usize {
    GLOBAL_ARENA.allocated_bytes()
}
