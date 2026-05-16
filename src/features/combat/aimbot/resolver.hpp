#ifndef AIMBOT_RESOLVER_HPP
#define AIMBOT_RESOLVER_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <cstring>

#include "aim_utils.hpp"

#include "core/shared/sigs.hpp"
#include "libsigscan/libsigscan.h"

namespace resolver
{

constexpr int max_entities = 65;
constexpr int max_records = 16;
constexpr int max_yaw_candidates = 24;

struct anim_state_snapshot {
  bool valid = false;
  float eye_yaw = 0.0f;
  float eye_pitch = 0.0f;
  float gait_yaw = 0.0f;
  float velocity_yaw = 0.0f;
  float speed_2d = 0.0f;
  bool moving = false;
};

struct resolver_record {
  bool valid = false;
  float sim_time = 0.0f;
  Vec3 origin{};
  Vec3 velocity{};
  float eye_yaw = 0.0f;
  float gait_yaw = 0.0f;
  float velocity_yaw = 0.0f;
  bool moving = false;
};

struct player_history {
  int ent_index = 0;
  int record_count = 0;
  std::array<resolver_record, max_records> records{};
};

struct yaw_candidate {
  bool valid = false;
  float yaw = 0.0f;
  float penalty = 0.0f;
};

struct yaw_candidate_list {
  int count = 0;
  std::array<yaw_candidate, max_yaw_candidates> values{};
};

inline std::array<player_history, max_entities> g_history{};

[[nodiscard]] inline float normalize_yaw(float yaw)
{
  return std::remainder(yaw, 360.0f);
}

[[nodiscard]] inline float yaw_delta(float left, float right)
{
  return normalize_yaw(left - right);
}

[[nodiscard]] inline bool finite_angle(float value)
{
  return std::isfinite(value) && value >= -720.0f && value <= 720.0f;
}

[[nodiscard]] inline float vector_yaw(const Vec3& value)
{
  return normalize_yaw(std::atan2(value.y, value.x) * radpi);
}

[[nodiscard]] inline float speed_2d(const Vec3& value)
{
  return std::sqrt((value.x * value.x) + (value.y * value.y));
}

[[nodiscard]] inline int player_anim_state_offset()
{
  static int offset = -1;
  if (offset >= 0) {
    return offset;
  }

  offset = 0;
  const auto* match = reinterpret_cast<const std::uint8_t*>(
    sigscan_module("client.so", sigs::ctf_player_anim_state_store));
  if (match == nullptr) {
    return offset;
  }

  std::int32_t store_offset = 0;
  std::memcpy(&store_offset, match + 13, sizeof(store_offset));
  if (store_offset >= 0x400 && store_offset <= 0x8000) {
    offset = store_offset;
  }

  return offset;
}

[[nodiscard]] inline void* player_anim_state(Player* player)
{
  const int offset = player_anim_state_offset();
  if (player == nullptr || offset <= 0) {
    return nullptr;
  }

  const auto base = reinterpret_cast<std::uintptr_t>(player);
  return *reinterpret_cast<void**>(base + static_cast<std::uintptr_t>(offset));
}

[[nodiscard]] inline bool read_anim_state_snapshot(Player* player, anim_state_snapshot* snapshot)
{
  if (snapshot == nullptr) {
    return false;
  }

  *snapshot = {};
  if (player == nullptr) {
    return false;
  }

  void* raw_state = player_anim_state(player);
  if (raw_state == nullptr) {
    return false;
  }

  const auto state = reinterpret_cast<std::uintptr_t>(raw_state);
  const auto owner_multi = *reinterpret_cast<void**>(state + 48);
  const auto owner_ctf = *reinterpret_cast<void**>(state + 304);
  if (owner_multi != player && owner_ctf != player) {
    return false;
  }

  Vec3 eye_angles = player->get_eye_angles();
  float eye_yaw = *reinterpret_cast<float*>(state + 140);
  const float ctf_eye_yaw = *reinterpret_cast<float*>(state + 60);
  if (!finite_angle(eye_yaw) && finite_angle(ctf_eye_yaw)) {
    eye_yaw = ctf_eye_yaw;
  }
  if (!finite_angle(eye_yaw)) {
    eye_yaw = eye_angles.y;
  }

  float eye_pitch = *reinterpret_cast<float*>(state + 144);
  if (!finite_angle(eye_pitch)) {
    eye_pitch = eye_angles.x;
  }

  float gait_yaw = *reinterpret_cast<float*>(state + 100);
  if (!finite_angle(gait_yaw)) {
    gait_yaw = eye_yaw;
  }

  const Vec3 velocity = player->get_velocity();
  const float movement_speed = speed_2d(velocity);

  snapshot->valid = true;
  snapshot->eye_yaw = normalize_yaw(eye_yaw);
  snapshot->eye_pitch = std::clamp(eye_pitch, -89.0f, 89.0f);
  snapshot->gait_yaw = normalize_yaw(gait_yaw);
  snapshot->velocity_yaw = movement_speed > 1.0f ? vector_yaw(velocity) : snapshot->gait_yaw;
  snapshot->speed_2d = movement_speed;
  snapshot->moving = movement_speed > 18.0f;
  return true;
}

[[nodiscard]] inline player_history* history_for_player(Player* player)
{
  if (player == nullptr) {
    return nullptr;
  }

  const int ent_index = player->get_index();
  if (ent_index <= 0 || ent_index >= max_entities) {
    return nullptr;
  }

  return &g_history[ent_index];
}

inline void record_player(Player* player)
{
  if (!config.aimbot.resolver || player == nullptr || player->is_dormant() || !player->is_alive()) {
    return;
  }

  player_history* history = history_for_player(player);
  if (history == nullptr) {
    return;
  }

  anim_state_snapshot snapshot{};
  if (!read_anim_state_snapshot(player, &snapshot)) {
    return;
  }

  resolver_record record{};
  record.valid = true;
  record.sim_time = player->get_simulation_time();
  record.origin = player->get_origin();
  record.velocity = player->get_velocity();
  record.eye_yaw = snapshot.eye_yaw;
  record.gait_yaw = snapshot.gait_yaw;
  record.velocity_yaw = snapshot.velocity_yaw;
  record.moving = snapshot.moving;

  if (!std::isfinite(record.sim_time) || record.sim_time <= 0.0f) {
    return;
  }

  history->ent_index = player->get_index();
  if (history->record_count > 0) {
    const resolver_record& last = history->records[0];
    if (last.valid &&
        std::fabs(last.sim_time - record.sim_time) <= 0.0001f &&
        std::fabs(yaw_delta(last.eye_yaw, record.eye_yaw)) <= 0.01f) {
      return;
    }
  }

  const int shift_count = std::min(history->record_count, max_records - 1);
  for (int index = shift_count; index > 0; --index) {
    history->records[index] = history->records[index - 1];
  }

  history->records[0] = record;
  history->record_count = std::min(history->record_count + 1, max_records);
}

inline void clear()
{
  g_history = {};
}

inline void add_yaw(yaw_candidate_list* list, float yaw, float penalty)
{
  if (list == nullptr || list->count >= max_yaw_candidates || !finite_angle(yaw)) {
    return;
  }

  const float normalized = normalize_yaw(yaw);
  for (int index = 0; index < list->count; ++index) {
    if (std::fabs(yaw_delta(list->values[index].yaw, normalized)) < 2.0f) {
      list->values[index].penalty = std::min(list->values[index].penalty, penalty);
      return;
    }
  }

  yaw_candidate& candidate = list->values[list->count];
  candidate.valid = true;
  candidate.yaw = normalized;
  candidate.penalty = penalty;
  ++list->count;
}

[[nodiscard]] inline float yaw_to_local(Player* localplayer, Player* player)
{
  if (localplayer == nullptr || player == nullptr) {
    return 0.0f;
  }

  return aimbot_calculate_angles_to_position(player->get_origin(), localplayer->get_origin()).y;
}

inline void add_history_yaws(Player* player, yaw_candidate_list* list)
{
  const player_history* history = history_for_player(player);
  if (history == nullptr || list == nullptr || history->record_count <= 0) {
    return;
  }

  const resolver_record& latest = history->records[0];
  if (latest.valid) {
    add_yaw(list, latest.eye_yaw, 2.0f);
    add_yaw(list, latest.gait_yaw, 3.0f);
    if (latest.moving) {
      add_yaw(list, latest.velocity_yaw, 1.0f);
    }
  }

  for (int index = 1; index < history->record_count; ++index) {
    const resolver_record& record = history->records[index];
    if (!record.valid) {
      continue;
    }

    if (record.moving) {
      add_yaw(list, record.velocity_yaw, 4.0f);
      add_yaw(list, record.gait_yaw, 5.0f);
      break;
    }
  }

  if (history->record_count >= 2 && history->records[0].valid && history->records[1].valid) {
    const float yaw_step = yaw_delta(history->records[0].eye_yaw, history->records[1].eye_yaw);
    if (std::fabs(yaw_step) <= 90.0f) {
      add_yaw(list, history->records[0].eye_yaw + yaw_step, 6.0f);
      add_yaw(list, history->records[0].gait_yaw + yaw_step, 7.0f);
    }
  }
}

[[nodiscard]] inline yaw_candidate_list build_yaw_candidates(Player* localplayer,
  Player* player,
  const anim_state_snapshot& snapshot)
{
  yaw_candidate_list list{};
  const Vec3 eye_angles = player != nullptr ? player->get_eye_angles() : Vec3{};
  const float eye_yaw = finite_angle(eye_angles.y) ? eye_angles.y : snapshot.eye_yaw;
  const float base_yaw = snapshot.valid ? snapshot.eye_yaw : eye_yaw;
  const float gait_yaw = snapshot.valid ? snapshot.gait_yaw : base_yaw;
  const float local_yaw = yaw_to_local(localplayer, player);

  add_yaw(&list, base_yaw, 0.0f);
  add_yaw(&list, gait_yaw, snapshot.moving ? 0.5f : 2.5f);
  if (snapshot.valid && snapshot.moving) {
    add_yaw(&list, snapshot.velocity_yaw, 0.25f);
  }

  add_history_yaws(player, &list);

  add_yaw(&list, local_yaw, 8.0f);
  add_yaw(&list, local_yaw + 180.0f, 8.5f);
  add_yaw(&list, local_yaw + 90.0f, 9.0f);
  add_yaw(&list, local_yaw - 90.0f, 9.0f);

  constexpr std::array<float, 8> desync_offsets = {
    -58.0f,
    58.0f,
    -90.0f,
    90.0f,
    -120.0f,
    120.0f,
    180.0f,
    -180.0f
  };

  for (const float offset : desync_offsets) {
    add_yaw(&list, base_yaw + offset, 10.0f + (std::fabs(offset) * 0.01f));
  }

  for (const float offset : desync_offsets) {
    add_yaw(&list, gait_yaw + offset, 11.0f + (std::fabs(offset) * 0.01f));
  }

  return list;
}

struct scoped_bone_cache_bypass {
  bool previous = false;
  bool changed = false;

  scoped_bone_cache_bypass()
  {
    previous = config.misc.exploits.setup_bones_optimization;
    changed = previous;
    if (changed) {
      config.misc.exploits.setup_bones_optimization = false;
    }
  }

  ~scoped_bone_cache_bypass()
  {
    if (changed) {
      config.misc.exploits.setup_bones_optimization = previous;
    }
  }
};

struct scoped_eye_angles {
  Player* player = nullptr;
  Vec3 original{};
  bool active = false;

  explicit scoped_eye_angles(Player* target_player)
    : player(target_player)
  {
    if (player != nullptr) {
      original = player->get_eye_angles();
      active = true;
    }
  }

  ~scoped_eye_angles()
  {
    if (active && player != nullptr) {
      player->set_eye_angles(original);
    }
  }

  void apply(float pitch, float yaw)
  {
    if (!active || player == nullptr) {
      return;
    }

    player->set_eye_angles(Vec3{ std::clamp(pitch, -89.0f, 89.0f), normalize_yaw(yaw), 0.0f });
  }
};

[[nodiscard]] inline float resolver_point_score(const aimbot_point& point, float yaw_penalty)
{
  return (static_cast<float>(point.priority) * 4096.0f) + point.fov + yaw_penalty;
}

[[nodiscard]] inline aimbot_point find_point(Player* localplayer,
  Weapon* weapon,
  Player* player,
  const Vec3& bullet_view_angles,
  bool require_visibility,
  unsigned int trace_mask)
{
  if (!config.aimbot.resolver || localplayer == nullptr || weapon == nullptr || player == nullptr) {
    return {};
  }

  anim_state_snapshot snapshot{};
  if (!read_anim_state_snapshot(player, &snapshot)) {
    return {};
  }

  yaw_candidate_list candidates = build_yaw_candidates(localplayer, player, snapshot);
  if (candidates.count <= 0) {
    return {};
  }

  const std::uint32_t configured_mask = config.aimbot.hitscan_hitboxes & aim_hitbox_mask_all;
  const std::uint32_t hitbox_mask = configured_mask != 0 ? configured_mask : aim_hitbox_mask_default_hitscan;
  const int max_candidates = std::clamp(config.aimbot.resolver_max_yaws, 4, max_yaw_candidates);
  const float pitch = finite_angle(snapshot.eye_pitch) ? snapshot.eye_pitch : player->get_eye_angles().x;

  scoped_bone_cache_bypass bone_cache_bypass{};
  scoped_eye_angles eye_angles(player);
  aimbot_point best_point{};
  float best_score = FLT_MAX;

  for (int index = 0; index < candidates.count && index < max_candidates; ++index) {
    const yaw_candidate& candidate = candidates.values[index];
    if (!candidate.valid) {
      continue;
    }

    eye_angles.apply(pitch, candidate.yaw);
    aimbot_point point = aimbot_find_best_point(
      localplayer,
      player,
      weapon,
      bullet_view_angles,
      hitbox_mask,
      require_visibility,
      trace_mask);
    if (!point.valid) {
      continue;
    }

    const float score = resolver_point_score(point, candidate.penalty);
    if (!best_point.valid || score < best_score) {
      best_point = point;
      best_score = score;
    }
  }

  return best_point;
}

} // namespace resolver

#endif
