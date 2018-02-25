#include <stdint.h>
#include "libk.h"
#include "util.h"

#define is_aligned(value, alignment) !(value & (alignment - 1))

struct source_location {
	const char *filename;
	uint32_t line;
	uint32_t column;
};

struct type_descriptor {
	uint16_t kind;
	uint16_t info;
	char name[];
};

struct type_mismatch_info {
	struct source_location location;
	struct type_descriptor *type;
	uintptr_t alignment;
	uint8_t type_check_kind;
};

struct overflow_data {
	struct source_location location;
	struct type_descriptor *type;
};

struct out_of_bounds_data {
	struct source_location location;
	struct type_descriptor *array_type;
	struct type_descriptor *index_type;
};

struct unreachable_data {
	struct source_location location;
};

struct invalid_value_data {
	struct source_location location;
	struct type_descriptor *type;
};

struct source_location unknown_location = {
	"<unknown file>", 0, 0
};

__attribute__((noreturn))
static void ubsan_abort(struct source_location *location, const char *message)
{
	if (!location || !location->filename)
		location = &unknown_location;
	panic("ubsan fatal error: %s at %s:%d\n", message, location->filename, location->line);
}

void __ubsan_handle_type_mismatch(struct type_mismatch_info *type_mismatch, uintptr_t pointer);
void __ubsan_handle_add_overflow(struct overflow_data *data, uintptr_t lhs, uintptr_t rhs);
void __ubsan_handle_sub_overflow(struct overflow_data *data, uintptr_t lhs, uintptr_t rhs);
void __ubsan_handle_negate_overflow(struct overflow_data *data, uintptr_t old_value);
void __ubsan_handle_divrem_overflow(struct overflow_data *data, uintptr_t lhs, uintptr_t rhs);
void __ubsan_handle_out_of_bounds(struct out_of_bounds_data *data, uintptr_t index);
void __ubsan_handle_builtin_unreachable(struct unreachable_data *data);
void __ubsan_handle_mul_overflow(struct overflow_data *data, uintptr_t lhs, uintptr_t rhs);
void __ubsan_handle_shift_out_of_bounds(struct out_of_bounds_data *data, uintptr_t lhs, uintptr_t rhs);
void __ubsan_handle_load_invalid_value(struct invalid_value_data *data, uintptr_t value);

void __ubsan_handle_type_mismatch(struct type_mismatch_info *type_mismatch, uintptr_t pointer)
{
	char *message;
	if (pointer == 0)
		message = "null pointer access";
	else if (type_mismatch->alignment != 0 && (pointer & (type_mismatch->alignment - 1)))
		//message = "unaligned memory access";
		return;
	else
		message = "type mismatch";
	ubsan_abort(&type_mismatch->location, message);
}

void __ubsan_handle_add_overflow(struct overflow_data *data, uintptr_t UNUSED(lhs), uintptr_t UNUSED(rhs))
{
	ubsan_abort(&data->location, "addition overflow");
}

void __ubsan_handle_sub_overflow(struct overflow_data *data, uintptr_t UNUSED(lhs), uintptr_t UNUSED(rhs))
{
	ubsan_abort(&data->location, "subtraction overflow");
}

void __ubsan_handle_negate_overflow(struct overflow_data *data, uintptr_t UNUSED(old_value))
{
	ubsan_abort(&data->location, "negation overflow");
}

void __ubsan_handle_divrem_overflow(struct overflow_data *data, uintptr_t UNUSED(lhs), uintptr_t UNUSED(rhs))
{
	ubsan_abort(&data->location, "division remainder overflow");
}

void __ubsan_handle_out_of_bounds(struct out_of_bounds_data *data, uintptr_t UNUSED(index))
{
	ubsan_abort(&data->location, "out of bounds");
}

void __ubsan_handle_builtin_unreachable(struct unreachable_data *data)
{
	ubsan_abort(&data->location, "reached builtin_unreachable");
}

void __ubsan_handle_mul_overflow(struct overflow_data *data, uintptr_t UNUSED(lhs), uintptr_t UNUSED(rhs))
{
	ubsan_abort(&data->location, "multiplication overflow");
}

void __ubsan_handle_shift_out_of_bounds(struct out_of_bounds_data *data, uintptr_t UNUSED(lhs), uintptr_t UNUSED(rhs))
{
	ubsan_abort(&data->location, "shift out of bounds");
}

void __ubsan_handle_load_invalid_value(struct invalid_value_data *data, uintptr_t UNUSED(value))
{
	ubsan_abort(&data->location, "invalid value load");
}
