/* Copyright (c) 2026 VillageSQL Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <villagesql/vsql.h>

#include <cctype>
#include <string_view>

// STRICTBOOL custom type for VillageSQL.
// Storage: 1 byte — 0x00 = FALSE, 0x01 = TRUE.
// Accepted input (case-insensitive): true/false, t/f, yes/no, on/off, 1/0.
// Output: "true" or "false".

using vsql::CustomArg;
using vsql::CustomResult;
using vsql::IntResult;
using vsql::RealResult;
using vsql::StringResult;
using vsql::INT;
using vsql::REAL;

namespace {

constexpr unsigned char kFalse = 0x00;
constexpr unsigned char kTrue  = 0x01;

// Case-insensitive comparison where b is always a lowercase ASCII literal.
bool iequal(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) != b[i])
      return false;
  }
  return true;
}

}  // namespace

void boolean_encode(std::string_view from, CustomResult out) try {
  auto buf = out.buffer();
  if (iequal(from, "true")  || iequal(from, "t") ||
      iequal(from, "yes")   || iequal(from, "on") ||
      iequal(from, "1")) {
    buf[0] = kTrue;
    out.set_length(1);
    return;
  }
  if (iequal(from, "false") || iequal(from, "f") ||
      iequal(from, "no")    || iequal(from, "off") ||
      iequal(from, "0")) {
    buf[0] = kFalse;
    out.set_length(1);
    return;
  }
  out.warning("invalid STRICTBOOL value; expected true/false/1/0/yes/no/on/off");
} catch (...) {
  out.error("STRICTBOOL encode: unexpected exception");
}

void boolean_decode(CustomArg in, StringResult out) try {
  if (in.is_null()) { out.set_null(); return; }
  auto src = in.value();
  out.set(src[0] == kTrue ? "true" : "false");
} catch (...) {
  out.error("STRICTBOOL decode: unexpected exception");
}

int boolean_compare(CustomArg a, CustomArg b) try {
  if (a.is_null() || b.is_null()) return 0;
  unsigned char av0 = a.value()[0];
  unsigned char bv0 = b.value()[0];
  if (av0 == bv0) return 0;
  return (av0 < bv0) ? -1 : 1;
} catch (...) {
  return 0;
}

size_t boolean_hash(CustomArg in) try {
  if (in.is_null()) return 0;
  return (in.value()[0] == kTrue) ? 1 : 0;
} catch (...) {
  return 0;
}

// =============================================================================
// Aggregate VDFs
// =============================================================================

// boolean_sum(STRICTBOOL) -> INT: count of TRUE values in the group.
struct BoolSumState {
  int64_t count = 0;
};

void boolean_sum_clear(BoolSumState &s) { s.count = 0; }

void boolean_sum_accumulate(BoolSumState &s, CustomArg v) {
  if (!v.is_null() && v.value()[0] == kTrue) ++s.count;
}

void boolean_sum_result(const BoolSumState &s, IntResult out) {
  out.set(s.count);
}

// boolean_avg(STRICTBOOL) -> REAL: ratio of TRUE values (true_count / total).
// Returns NULL when the group is empty.
struct BoolAvgState {
  int64_t true_count = 0;
  int64_t total = 0;
};

void boolean_avg_clear(BoolAvgState &s) {
  s.true_count = 0;
  s.total = 0;
}

void boolean_avg_accumulate(BoolAvgState &s, CustomArg v) {
  if (!v.is_null()) {
    ++s.total;
    if (v.value()[0] == kTrue) ++s.true_count;
  }
}

void boolean_avg_result(const BoolAvgState &s, RealResult out) {
  if (s.total == 0) { out.set_null(); return; }
  out.set(static_cast<double>(s.true_count) / static_cast<double>(s.total));
}

// =============================================================================
// Type
// =============================================================================

constexpr const char kBooleanTypeName[] = "STRICTBOOL";

constexpr auto STRICTBOOL = vsql::make_type<kBooleanTypeName>()
    .persisted_length(1)
    .max_decode_buffer_length(6)
    .from_string<&boolean_encode>()
    .to_string<&boolean_decode>()
    .compare<&boolean_compare>()
    .hash<&boolean_hash>()
    .intrinsic_default_str("false")
    .build();

VEF_GENERATE_ENTRY_POINTS(
  vsql::make_extension()
    .type(STRICTBOOL)
    .func(vsql::make_aggregate_func<BoolSumState, &boolean_sum_result>(
              "boolean_sum")
              .returns(INT)
              .param("STRICTBOOL")
              .clear<&boolean_sum_clear>()
              .accumulate<&boolean_sum_accumulate>()
              .build())
    .func(vsql::make_aggregate_func<BoolAvgState, &boolean_avg_result>(
              "boolean_avg")
              .returns(REAL)
              .param("STRICTBOOL")
              .clear<&boolean_avg_clear>()
              .accumulate<&boolean_avg_accumulate>()
              .build())
)
