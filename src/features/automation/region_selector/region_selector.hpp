/*
/^-----^\   data: 2026-05-06
V  o o  V  file: src/features/automation/region_selector/region_selector.hpp
 |  Y  |   author: pupnoodle
  \ Q /
  / - \
  |    \
  |     \     )
  || (___\====
*/

#ifndef REGION_SELECTOR_HPP
#define REGION_SELECTOR_HPP

#include <array>
#include <cstdint>
#include <string_view>

#include "games/tf2/sdk/interfaces/steam_networking_utils.hpp"

namespace automation::region_selector
{

struct data_center
{
  const char* continent;
  const char* label;
  const char* code;
  std::uint64_t bit;
};

constexpr std::array<data_center, 47> data_centers{{
  // europe
  {"Europe", "amsterdam (ams)", "ams", 1ull << 0},
  {"Europe", "frankfurt (fra)", "fra", 1ull << 1},
  {"Europe", "falkenstein (fsn)", "fsn", 1ull << 2},
  {"Europe", "helsinki (hel)", "hel", 1ull << 3},
  {"Europe", "london (lhr)", "lhr", 1ull << 4},
  {"Europe", "luxembourg (lux)", "lux", 1ull << 10},
  {"Europe", "luxembourg 1 (lux1)", "lux1", 1ull << 11},
  {"Europe", "luxembourg 2 (lux2)", "lux2", 1ull << 12},
  {"Europe", "madrid (mad)", "mad", 1ull << 5},
  {"Europe", "paris (par)", "par", 1ull << 6},
  {"Europe", "stockholm 1 (sto)", "sto", 1ull << 7},
  {"Europe", "stockholm 2 (sto2)", "sto2", 1ull << 8},
  {"Europe", "vienna (vie)", "vie", 1ull << 13},
  {"Europe", "warsaw (waw)", "waw", 1ull << 9},

  // north america
  {"North America", "atlanta (atl)", "atl", 1ull << 14},
  {"North America", "chicago (ord)", "ord", 1ull << 20},
  {"North America", "dallas (dfw)", "dfw", 1ull << 22},
  {"North America", "los angeles (lax)", "lax", 1ull << 18},
  {"North America", "moses lake (eat)", "eat", 1ull << 15},
  {"North America", "moses lake 2 (mwh)", "mwh", 1ull << 16},
  {"North America", "oklahoma city (okc)", "okc", 1ull << 19},
  {"North America", "seattle (sea)", "sea", 1ull << 21},
  {"North America", "virginia (iad)", "iad", 1ull << 17},

  // south america
  {"South America", "buenos aires (eze)", "eze", 1ull << 26},
  {"South America", "lima (lim)", "lim", 1ull << 24},
  {"South America", "santiago (scl)", "scl", 1ull << 25},
  {"South America", "sao paulo (gru)", "gru", 1ull << 23},

  // asia
  {"Asia", "bangalore (maa3)", "maa3", 1ull << 33},
  {"Asia", "chennai/ambattur (maa)", "maa", 1ull << 31},
  {"Asia", "dubai (dxb)", "dxb", 1ull << 28},
  {"Asia", "hong kong (hkg)", "hkg", 1ull << 30},
  {"Asia", "manila (man)", "man", 1ull << 34},
  {"Asia", "mumbai (bom)", "bom", 1ull << 27},
  {"Asia", "new delhi (maa2)", "maa2", 1ull << 32},
  {"Asia", "seoul (seo)", "seo", 1ull << 39},
  {"Asia", "singapore (sgp)", "sgp", 1ull << 35},
  {"Asia", "tokyo 1 (tyo)", "tyo", 1ull << 36},
  {"Asia", "tokyo 2 (tyo1)", "tyo1", 1ull << 38},
  {"Asia", "tokyo 3 (tyo2)", "tyo2", 1ull << 37},
  {"Asia", "tokyo narita (gnrt)", "gnrt", 1ull << 29},

  // china
  {"China", "chengdu (ctu)", "ctu", 1ull << 44},
  {"China", "guangzhou (can)", "can", 1ull << 40},
  {"China", "shanghai (sha)", "sha", 1ull << 41},
  {"China", "tianjin (tsn)", "tsn", 1ull << 42},
  {"China", "wuhan (wuh)", "wuh", 1ull << 43},

  // oceania
  {"Oceania", "sydney (syd)", "syd", 1ull << 45},

  // africa
  {"Africa", "johannesburg (jnb)", "jnb", 1ull << 46},
}};

constexpr std::uint64_t all_region_bits = (1ull << 47) - 1ull;
constexpr int blocked_region_ping = 69420;
constexpr int preferred_region_ping = 1;

bool is_region_allowed(std::string_view region);
void set_region_allowed(std::uint64_t bit, bool allowed);
bool is_region_bit_allowed(std::uint64_t bit);
int adjust_ping(int original_ping, steam_networking_pop_id pop_id);
void refresh_ping_data();

} // namespace automation::region_selector

#endif
