#ifndef VISUAL_GROUPS_HPP
#define VISUAL_GROUPS_HPP

#include "features/menu/config.hpp"

class Entity;
class Player;

namespace visual_groups
{

void ensure_defaults();
void store(Player* localplayer);
[[nodiscard]] const visual_group* group_for_entity(Entity* entity, bool models = true);
[[nodiscard]] bool groups_active();
void move_group(int from, int to);
[[nodiscard]] RGBA_float color_for_entity(Entity* entity, const visual_group& group);
[[nodiscard]] float alpha_for_entity(Entity* entity, float start, float end, bool smooth_alpha);
[[nodiscard]] const char* label_for_entity(Entity* entity);

} 

#endif
