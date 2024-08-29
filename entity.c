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
	Sprite_ID sprite_id;
    bool is_animate;
	float32 health, damage;
    Vector2 hitbox[4];
    Entity_Archetype primary_weapon;
    Entity_Archetype secondary_weapon;
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

Vector2* entity_resolve_hitbox(Entity* en) {
    Vector2* hitbox = alloc(get_temporary_allocator(), sizeof(Vector2) * 4);
    hitbox[0] = v2_add(en->hitbox[0], en->pos);
	hitbox[1] = v2_add(en->hitbox[1], en->pos);
	hitbox[2] = v2_add(en->hitbox[2], en->pos);
	hitbox[3] = v2_add(en->hitbox[3], en->pos);
    return hitbox;
}