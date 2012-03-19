#include "db_vm.h"

FLASH_SPACE char str_null[]                   = "";
FLASH_SPACE char str_open_failed_err[]        = "open failed: %s";
FLASH_SPACE char str_run_failed_err[]         = "run failed: %s";
FLASH_SPACE char str_load_failed_err[]        = "load failed";
FLASH_SPACE char str_string_index_range_err[] = "string index out of range: %d";
FLASH_SPACE char str_no_heap_err[]            = "no string heap available";
FLASH_SPACE char str_no_heap_handles_err[]    = "insufficient string heap handles";
FLASH_SPACE char str_no_heap_space_err[]      = "insufficient string heap space";
FLASH_SPACE char str_heap_debug_output[]      = "%04x h: %04x, c: %d, l: %d, d: %04x '";
FLASH_SPACE char str_heap_debug_output_tail[] = "'\n";
FLASH_SPACE char str_array_subscript_err[]    = "array subscript out of bounds: %d";
FLASH_SPACE char str_array_subscript1_err[]   = "first array subscript out of bounds: %d";
FLASH_SPACE char str_array_subscript2_err[]   = "second array subscript out of bounds: %d";
FLASH_SPACE char str_image_header_err[]       = "header verify failed\n";
FLASH_SPACE char str_image_target_err[]       = "not built for this target: expected %d, got %d\n"; // (unused)
FLASH_SPACE char str_data_space_err[]         = "data space allocation failed: data %u, heap %u\n";
FLASH_SPACE char str_opcode_err[]             = "undefined opcode 0x%02x";
FLASH_SPACE char str_stack_overflow_err[]     = "stack overflow";
FLASH_SPACE char str_object_number_err[]      = "invalid object number: %d";
FLASH_SPACE char str_tag_not_found_err[]      = "tag not found: %d";
FLASH_SPACE char str_not_code_object_err[]    = "not code object: %d";
FLASH_SPACE char str_argument_count_err[]     = "wrong number of arguments";
FLASH_SPACE char str_wrong_type_err[]         = "wrong type";
FLASH_SPACE char str_warn_prefix[]            = "warning: ";
FLASH_SPACE char str_abort_prefix[]           = "abort: ";
FLASH_SPACE char str_program_id[]             = "XGS Basic Runtime %d.%d\n";
FLASH_SPACE char str_ready_for_download[]     = "ready for download...";


