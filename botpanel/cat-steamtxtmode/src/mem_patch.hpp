#ifndef CAT_STEAMTXTMODE_MEM_PATCH_HPP
#define CAT_STEAMTXTMODE_MEM_PATCH_HPP

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sys/mman.h>
#include <unistd.h>

namespace cat_stm
{

struct module_range
{
  std::uintptr_t start = 0;
  std::uintptr_t end = 0;
};

inline std::vector<module_range> module_exec_ranges(std::string_view module_basename)
{
  std::vector<module_range> ranges;
  std::FILE* maps = std::fopen("/proc/self/maps", "re");
  if (maps == nullptr)
  {
    return ranges;
  }

  char line[1024];
  while (std::fgets(line, sizeof(line), maps) != nullptr)
  {
    unsigned long start = 0;
    unsigned long end = 0;
    char perms[5] = { 0 };
    int path_offset = 0;
    if (std::sscanf(line, "%lx-%lx %4s %*x %*x:%*x %*u %n", &start, &end, perms, &path_offset) < 3)
    {
      continue;
    }
    if (perms[2] != 'x')
    {
      continue;
    }

    const char* path = line + path_offset;
    while (*path == ' ')
    {
      ++path;
    }
    std::string_view path_view{ path };
    if (!path_view.empty() && path_view.back() == '\n')
    {
      path_view.remove_suffix(1);
    }
    const auto slash = path_view.find_last_of('/');
    const std::string_view base = (slash == std::string_view::npos) ? path_view : path_view.substr(slash + 1);
    if (base == module_basename)
    {
      ranges.push_back({ static_cast<std::uintptr_t>(start), static_cast<std::uintptr_t>(end) });
    }
  }

  std::fclose(maps);
  return ranges;
}

struct sig_byte
{
  std::uint8_t value = 0;
  bool wildcard = false;
};

inline std::vector<sig_byte> parse_signature(std::string_view pattern)
{
  std::vector<sig_byte> bytes;
  std::size_t index = 0;
  while (index < pattern.size())
  {
    const char c = pattern[index];
    if (c == ' ')
    {
      ++index;
      continue;
    }
    if (c == '?')
    {
      bytes.push_back({ 0, true });
      ++index;
      if (index < pattern.size() && pattern[index] == '?')
      {
        ++index;
      }
      continue;
    }
    const auto hex_value = [](char ch) -> int
    {
      if (ch >= '0' && ch <= '9') return ch - '0';
      if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
      if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
      return -1;
    };
    const int hi = hex_value(c);
    const int lo = (index + 1 < pattern.size()) ? hex_value(pattern[index + 1]) : -1;
    if (hi < 0 || lo < 0)
    {
      break;
    }
    bytes.push_back({ static_cast<std::uint8_t>((hi << 4) | lo), false });
    index += 2;
  }
  return bytes;
}

inline std::uint8_t* scan_module(std::string_view module_basename, std::string_view pattern)
{
  const std::vector<sig_byte> sig = parse_signature(pattern);
  if (sig.empty())
  {
    return nullptr;
  }

  for (const module_range& range : module_exec_ranges(module_basename))
  {
    auto* base = reinterpret_cast<std::uint8_t*>(range.start);
    const std::size_t span = range.end - range.start;
    if (span < sig.size())
    {
      continue;
    }
    for (std::size_t offset = 0; offset + sig.size() <= span; ++offset)
    {
      bool match = true;
      for (std::size_t k = 0; k < sig.size(); ++k)
      {
        if (!sig[k].wildcard && base[offset + k] != sig[k].value)
        {
          match = false;
          break;
        }
      }
      if (match)
      {
        return base + offset;
      }
    }
  }
  return nullptr;
}

class byte_patch
{
public:
  byte_patch() = default;

  byte_patch(void* address, std::vector<std::uint8_t> replacement)
    : address_(address), replacement_(std::move(replacement))
  {
  }

  bool valid() const
  {
    return address_ != nullptr && !replacement_.empty();
  }

  bool apply()
  {
    if (!valid() || applied_)
    {
      return applied_;
    }
    if (!set_protection(true))
    {
      return false;
    }
    original_.assign(static_cast<std::uint8_t*>(address_),
                     static_cast<std::uint8_t*>(address_) + replacement_.size());
    std::memcpy(address_, replacement_.data(), replacement_.size());
    set_protection(false);
    applied_ = true;
    return true;
  }

  void restore()
  {
    if (!applied_)
    {
      return;
    }
    if (set_protection(true))
    {
      std::memcpy(address_, original_.data(), original_.size());
      set_protection(false);
    }
    applied_ = false;
  }

private:
  bool set_protection(bool writable)
  {
    const auto page = static_cast<std::uintptr_t>(::sysconf(_SC_PAGESIZE));
    const auto start = reinterpret_cast<std::uintptr_t>(address_);
    const auto end = start + replacement_.size();
    const auto aligned_start = start & ~(page - 1);
    const auto aligned_end = (end + page - 1) & ~(page - 1);
    const int prot = writable ? (PROT_READ | PROT_WRITE | PROT_EXEC) : (PROT_READ | PROT_EXEC);
    return ::mprotect(reinterpret_cast<void*>(aligned_start),
                      aligned_end - aligned_start, prot) == 0;
  }

  void* address_ = nullptr;
  std::vector<std::uint8_t> replacement_;
  std::vector<std::uint8_t> original_;
  bool applied_ = false;
};

}

#endif
