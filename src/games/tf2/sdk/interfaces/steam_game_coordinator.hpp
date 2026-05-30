#ifndef STEAM_GAME_COORDINATOR_HPP
#define STEAM_GAME_COORDINATOR_HPP

#include <cstdint>

class steam_game_coordinator
{
public:
  bool send_message(std::uint32_t message_type, const void* data, std::uint32_t data_size)
  {
    void** vtable = *reinterpret_cast<void***>(this);
    int (*send_message_fn)(void*, std::uint32_t, const void*, std::uint32_t) =
      reinterpret_cast<int (*)(void*, std::uint32_t, const void*, std::uint32_t)>(vtable[0]);
    return send_message_fn(this, message_type, data, data_size) == 0;
  }
};

inline static steam_game_coordinator* steam_game_coordinator_interface;

#endif
