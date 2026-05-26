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
using vsql::StringResult;

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
)
