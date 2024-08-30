typedef enum Weapon_Class {
    WEAPON_nil = 0,
    WEAPON_melee = 1,
    WEAPON_ranged = 2,
    WEAPON_magic = 3,
} Weapon_Class;

typedef enum Entity_Archetype {
	ARCH_nil = 0,
	ARCH_player = 1,
	ARCH_spider = 2,
	ARCH_mutant = 3,
	ARCH_tree = 4,
	ARCH_startersword = 5,
	ARCH_starterbow = 6,
    ARCH_arrow = 7,
} Entity_Archetype;

typedef struct Entity {
	bool alive;
	bool renderable;
	Entity_Archetype arch;
	Vector2 pos;
    bool is_animate, is_hostile;
    float32 enemy_cooldown_secs;
    float32 enemy_speed;
	float32 max_health, health, damage;
    Vector2 hitbox[4];
    Entity_Archetype primary_weapon;
    Entity_Archetype secondary_weapon;
    Armour armour;
    bool is_weapon;
    Weapon_Class weapon_class;
	Vector2 weapon_owner_pos;
	Vector2 weapon_dir;
	float32 weapon_rads;
    float32 weapon_speed;
    float32 weapon_cooldown_secs;
    bool is_item;
    Vector2 item_inv_size;
    bool is_projectile;
    float32 projectile_range;
    Vector2 projectile_fire_dir;
    Vector2 projectile_start_pos;
    float32 projectile_speed;
} Entity;

#define MAX_ENTITIES 1024
typedef struct World {
	Entity entities[MAX_ENTITIES];
} World;
World* world = 0;

Entity* entity_create() {
	Entity* entity_found = 0;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity* existing_entity = &world->entities[i];
		if (!existing_entity->alive) {
			entity_found = existing_entity;
			break;
		}
	}
	assert(entity_found, "Entity Overflow!");
	entity_found->alive = true;
	return entity_found;
}

void entity_destroy(Entity* entity) {
	memset(entity, 0, sizeof(Entity));
}

bool entity_increment(Entity **entity_ptr)
{
  Entity *entity = *entity_ptr;
  int start_index = 0;
  if(entity != 0)
  {
    start_index = (entity - world->entities) + 1;
  }
  entity = 0;
  for(int idx = start_index; idx < MAX_ENTITIES; idx += 1)
  {
    entity = &world->entities[idx];
    if(entity->alive)
    {
      break;
    }
  }
  *entity_ptr = entity;
  return !!entity;
}

Sprite_ID entity_sprite_id_from_arch(Entity_Archetype arch) {
	switch (arch) {
		case ARCH_player: return SPRITE_player; break;
        case ARCH_spider: return SPRITE_spider ; break;
        case ARCH_mutant: return SPRITE_mutant ; break;
        case ARCH_tree: return SPRITE_tree ; break;
        case ARCH_startersword: return SPRITE_startersword ; break;
        case ARCH_starterbow: return SPRITE_starterbow ; break;
        case ARCH_arrow: return SPRITE_arrow ; break;
        default: return 0;
	}
}

Vector2* entity_resolve_hitbox(Entity* en) {
    Vector2* hitbox = alloc(get_temporary_allocator(), sizeof(Vector2) * 4);
    Sprite* sprite = sprite_get(entity_sprite_id_from_arch(en->arch));
    hitbox[0] = v2_add(en->hitbox[0], v2_add(en->pos, v2(sprite->size.x * -0.5, 0)));
    hitbox[1] = v2_add(en->hitbox[1], v2_add(en->pos, v2(sprite->size.x * -0.5, 0)));
    hitbox[2] = v2_add(en->hitbox[2], v2_add(en->pos, v2(sprite->size.x * -0.5, 0)));
    hitbox[3] = v2_add(en->hitbox[3], v2_add(en->pos, v2(sprite->size.x * -0.5, 0)));
    return hitbox;
}

void entity_take_damage(Entity* en, float damage) {
    damage *= armour_damage_multiplier(en->armour);
    en->health = clamp(en->health - damage, 0, en->max_health);
    // todo add knockback
}