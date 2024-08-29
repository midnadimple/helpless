typedef enum Entity_Archetype {
	ARCH_nil = 0,
	ARCH_player = 1,
	ARCH_spider = 2,
	ARCH_mutant = 3,
	ARCH_tree = 4,
	ARCH_weapon = 5,
} Entity_Archetype;

typedef struct Entity {
	bool alive;
	bool renderable;
	Entity_Archetype arch;
	Vector2 pos;
	Sprite_ID sprite_id;
	float32 health, damage;
    Vector2 hitbox[4];
	Vector2 weapon_owner_pos;
	Vector2 weapon_dir;
	float32 weapon_rads;
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