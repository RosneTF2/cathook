#include "config.hpp"
#include "mem_patch.hpp"

#include <vector>

namespace cat_stm
{

namespace
{

struct patch_def
{
  const char* module;
  const char* signature;
  int offset;
  std::vector<std::uint8_t> bytes;
  const char* name;
};

const std::vector<patch_def>& patch_table()
{
  static const std::vector<patch_def> table = {
    {
      "libcef.so",
      "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC 58 01 00 00 41 89 FF 4C 8B 6D 10 49 BE ?? ?? ?? ?? ?? ?? ?? ?? 48 8D BD 80 FE FF FF 4C 89 77 10 0F 28 05 ?? ?? ?? ?? 0F 29 07",
      0,
      { 0x31, 0xC0, 0xC3 },
      "libcef_crashpad_client_start"
    },
  };
  return table;
}

std::vector<byte_patch>& live_patches()
{
  static std::vector<byte_patch> patches;
  return patches;
}

}

void apply_steam_patches()
{
  const std::vector<patch_def>& table = patch_table();
  if (table.empty())
  {
    log_line("patches: table empty");
    return;
  }

  int applied = 0;
  for (const patch_def& def : table)
  {
    std::uint8_t* match = scan_module(def.module, def.signature);
    if (match == nullptr)
    {
      log_line("patch skip name=%s module=%s", def.name, def.module);
      continue;
    }

    byte_patch patch(match + def.offset, def.bytes);
    if (!patch.apply())
    {
      log_line("patch fail name=%s module=%s", def.name, def.module);
      continue;
    }

    live_patches().push_back(std::move(patch));
    ++applied;
    log_line("patch ok name=%s module=%s", def.name, def.module);
  }

  log_line("patches: applied %d/%zu", applied, table.size());
}

}
