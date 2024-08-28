typedef struct Sprite {
	Gfx_Image* image;
	Vector2 size;
} Sprite;

typedef enum Sprite_ID {
	SPRITE_player,
	SPRITE_spider,
	SPRITE_mutant,
	SPRITE_tree,
	SPRITE_MAX, 
} Sprite_ID;
Sprite sprites[SPRITE_MAX];

Sprite* sprite_get(Sprite_ID id) {
	if (id >= 0 && id < SPRITE_MAX) {
		return &sprites[id];
	}
	return &sprites[0];
}

typedef enum Entity_Archetype {
	ARCH_nil = 0,
	ARCH_player = 1,
	ARCH_spider = 2,
	ARCH_mutant = 3,
	ARCH_tree = 4,
} Entity_Archetype;

typedef struct Entity {
	bool alive;
	Entity_Archetype arch;
	Vector2 pos;
	Sprite_ID sprite_id;
	bool renderable;	
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

void player_setup(Entity* en) {
	en->arch = ARCH_player;
	en->sprite_id = SPRITE_player;
}

void spider_setup(Entity* en) {
	en->arch = ARCH_spider;
	en->sprite_id = SPRITE_spider;
	en->pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
}

int entry(int argc, char **argv) {
	
	// This is how we (optionally) configure the window.
	// You can set this at any point in the runtime and it will
	// be applied in os_update().
	// If you don't care, you can ignore all of this as it all
	// has reasonable default values.
	window.title = STR("Helpless");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.clear_color = hex_to_rgba(0x222034ff);
	window.allow_resize = false;
	window.fullscreen = true;

	world = alloc(get_heap_allocator(), sizeof(World));

	Gfx_Image *player_image = load_image_from_disk(STR("assets/player.png"), get_heap_allocator());
	assert(player_image, "Player image not found D:");
	sprites[SPRITE_player].image = player_image;
	sprites[SPRITE_player].size = v2((float32)player_image->width, (float32)player_image->height);

	Gfx_Image *spider_image = load_image_from_disk(STR("assets/spider.png"), get_heap_allocator());
	assert(spider_image, "Spider image not found D:");
	sprites[SPRITE_spider].image = spider_image;
	sprites[SPRITE_spider].size = v2((float32)spider_image->width, (float32)spider_image->height);

	Entity* player = entity_create();
	player_setup(player);

	for (int i = 0; i < 10; i++) {
		Entity* spider = entity_create();
		spider_setup(spider);
	}

	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		float64 now_time = os_get_elapsed_seconds();
		float64 delta_time = now_time - last_time;
		last_time = now_time;

		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		
		float64 zoom = 5.3;
		draw_frame.camera_xform = m4_make_scale(v3(1.0/zoom, 1.0/zoom, 1.0));		

		if (is_key_just_pressed('F')) {
			window.fullscreen = !window.fullscreen;
		}

		if (is_key_just_pressed(KEY_ESCAPE)) {
			window.should_close = true;
		}

		Vector2 move_axis = v2(0.0, 0.0);
		if (is_key_down('W')) {
			move_axis.y += 1.0;
		}
		if (is_key_down('A')) {
			move_axis.x -= 1.0;
		}
		if (is_key_down('S')) {
			move_axis.y -= 1.0;
		}
		if (is_key_down('D')) {
			move_axis.x += 1.0;
		}
		move_axis = v2_normalize(move_axis);

		player->pos = v2_add(player->pos, v2_mulf(move_axis, 30 * delta_time));

		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity* en = &world->entities[i];
			if (en->alive && en->renderable) {
				switch (en->arch) {
					default: {
						Sprite* sprite = sprite_get(en->sprite_id);

						Matrix4 xform = m4_scalar(1.0);
						xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
						xform = m4_translate(xform, v3(sprite->size.x * -0.5, 0, 0));
						draw_image_xform(sprite->image, xform, sprite->size, COLOR_WHITE);
					}
				}
			}
		}
		
		os_update(); 
		gfx_update();

		reset_temporary_storage();
	}

	return 0;
}