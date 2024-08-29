#include "util.c"
#include "sprite.c"
#include "entity.c"
#include "gjk.c"

void player_setup(Entity* en) {
	en->arch = ARCH_player;
	en->sprite_id = SPRITE_player;
	en->renderable = true;
	en->health = 100;
}

void spider_setup(Entity* en) {
	en->arch = ARCH_spider;
	en->sprite_id = SPRITE_spider;
	en->renderable = true;
	en->health = 30;
	en->pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
	en->hitbox[0] = v2(0, 0);
	en->hitbox[1] = v2(0, 7);
	en->hitbox[2] = v2(20, 0);
	en->hitbox[3] = v2(20, 7);
}

void startersword_setup(Entity* en) {
	en->arch = ARCH_weapon;
	en->sprite_id = SPRITE_startersword;
	en->renderable = true;
	en->damage = 10;
	en->hitbox[0] = v2(0, 0);
	en->hitbox[1] = v2(0, 13);
	en->hitbox[2] = v2(13, 0);
	en->hitbox[3] = v2(13, 13);
}

int entry(int argc, char **argv) {
	seed_for_random = rdtsc();
	
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
	window.fullscreen = false;

	world = alloc(get_heap_allocator(), sizeof(World));

	sprite_load(STR("assets/player.png"), SPRITE_player);
	sprite_load(STR("assets/spider.png"), SPRITE_spider);
	sprite_load(STR("assets/startersword.png"), SPRITE_startersword);

	Entity* player = entity_create();
	player_setup(player);

	Entity* startersword = entity_create();
	startersword_setup(startersword);

	for (int i = 0; i < 10; i++) {
		Entity* spider = entity_create();
		spider_setup(spider);
	}

	float64 zoom = 5.3;
	Vector2 camera_pos = v2(0, 0);

	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		float64 now_time = os_get_elapsed_seconds();
		float64 delta_time = now_time - last_time;
		last_time = now_time;

		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		
		// :camera
		{
			Vector2 target_pos = player->pos;
			animate_to_target_v2(&camera_pos, target_pos, delta_time, 7.5f);

			draw_frame.camera_xform = m4_identity();
			draw_frame.camera_xform = m4_translate(draw_frame.camera_xform, v3(camera_pos.x, camera_pos.y, 0.0));
			draw_frame.camera_xform = m4_scale(draw_frame.camera_xform, v3(1.0/zoom, 1.0/zoom, 1.0));
		}		

		if (is_key_just_pressed('F')) {
			window.fullscreen = !window.fullscreen;
		}

		if (is_key_just_pressed(KEY_ESCAPE)) {
			window.should_close = true;
		}

		Vector2 move_axis = v2(0.0, 0.0);
		if (is_key_down(KEY_ARROW_UP)) {
			move_axis.y += 1.0;
		}
		if (is_key_down(KEY_ARROW_LEFT)) {
			move_axis.x -= 1.0;
		}
		if (is_key_down(KEY_ARROW_DOWN)) {
			move_axis.y -= 1.0;
		}
		if (is_key_down(KEY_ARROW_RIGHT)) {
			move_axis.x += 1.0;
		}
		move_axis = v2_normalize(move_axis);

		player->pos = v2_add(player->pos, v2_mulf(move_axis, 75 * delta_time));
		startersword->weapon_owner_pos = player->pos;
		if (move_axis.x != 0 || move_axis.y != 0) {
			startersword->weapon_dir = move_axis;
		}

		// :generic simulation (i.e. not a specific boss or player)
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity* en = &world->entities[i];
			if (en->alive) {
				switch (en->arch) {
					case ARCH_weapon: {
						if (is_key_just_pressed('Z')) {
							for (int i = 0; i < MAX_ENTITIES; i++) {
								Entity* other_en = &world->entities[i];

								Vector2 hitbox[4] = {
									v2_add(en->hitbox[0], en->pos),
									v2_add(en->hitbox[1], en->pos),
									v2_add(en->hitbox[2], en->pos),
									v2_add(en->hitbox[3], en->pos),
								};

								Vector2 other_hitbox[4] = {
									v2_add(other_en->hitbox[0], other_en->pos),
									v2_add(other_en->hitbox[1], other_en->pos),
									v2_add(other_en->hitbox[2], other_en->pos),
									v2_add(other_en->hitbox[3], other_en->pos),
								};

								if (other_en->alive && gjk(hitbox, 4, other_hitbox, 4)) {
									other_en->health -= en->damage;
								}
							}
						}
					} break;

					default: {
						if (en->health <= 0) {
							en->alive = false;
						}
					} break;
				}
			}
		}

		// :rendering
		// TODO add a layer system, so that you place render instructions anywhere
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity* en = &world->entities[i];
			if (en->alive && en->renderable) {
				switch (en->arch) {
					case ARCH_weapon: {
						Sprite* sprite = sprite_get(en->sprite_id);

						// move the weapon away from its owner
						Vector2 dist_from_owner = v2_mulf(sprite->size, 1.25f);
						dist_from_owner.x = max(20, dist_from_owner.x);
						dist_from_owner.y = max(20, dist_from_owner.y);

						Vector2 target_pos = v2_add(en->weapon_owner_pos, v2_mul(en->weapon_dir, dist_from_owner));
						animate_to_target_v2(&(en->pos), target_pos, delta_time, 30.0f);

						float32 target_rads = atan2(en->weapon_dir.x, en->weapon_dir.y) - 45 * RAD_PER_DEG;
						animate_to_target_f32(&(en->weapon_rads), target_rads, delta_time, 15.0f);

						Matrix4 xform = m4_scalar(1.0);
						xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
						xform = m4_rotate_z(xform, en->weapon_rads);

						xform = m4_translate(xform, v3(sprite->size.x * -0.5, sprite->size.y * -0.5, 0));
						draw_image_xform(sprite->image, xform, sprite->size, COLOR_WHITE);

					} break;

					default: {
						Sprite* sprite = sprite_get(en->sprite_id);

						Matrix4 xform = m4_scalar(1.0);
						xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));

						xform = m4_translate(xform, v3(sprite->size.x * -0.5, sprite->size.y * -0.5, 0));
						draw_image_xform(sprite->image, xform, sprite->size, COLOR_WHITE);

					} break;
				}
			}
		}
		
		os_update(); 
		gfx_update();

		reset_temporary_storage();
	}

	return 0;
}