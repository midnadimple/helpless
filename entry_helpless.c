#include "util.c"
#include "sprite.c"
#include "entity.c"
#include "gjk.c"

void player_setup(Entity* en) {
	en->arch = ARCH_player;
	en->sprite_id = SPRITE_player;
	en->renderable = true;
	en->is_animate = true;
	en->health = 100;
}

void spider_setup(Entity* en) {
	en->arch = ARCH_spider;
	en->sprite_id = SPRITE_spider;
	en->renderable = true;
	en->is_animate = true;
	en->health = 30;
	en->pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
	en->hitbox[0] = v2(0, 0);
	en->hitbox[1] = v2(0, 7);
	en->hitbox[2] = v2(20, 0);
	en->hitbox[3] = v2(20, 7);
}

void weapon_setup(Entity* en, Entity_Archetype weapon_type) {
	en->arch = weapon_type;
	en->renderable = true;
	en->is_weapon = true;
	switch (weapon_type) {
		case ARCH_startersword: {
			en->sprite_id = SPRITE_startersword;
			en->weapon_class = WEAPON_melee;
			en->damage = 10;
			en->hitbox[0] = v2(0, 0);
			en->hitbox[1] = v2(0, 13);
			en->hitbox[2] = v2(13, 0);
			en->hitbox[3] = v2(13, 13);
			en->weapon_speed = 3;
		} break;

		case ARCH_starterbow: {
			en->sprite_id = SPRITE_starterbow;
			en->weapon_class = WEAPON_ranged;
			en->damage = 5;
			en->weapon_speed = 3;
		} break;

		default: {
			assert(false, "Invalid weapon type");
		} break;
	}
}

void arrow_setup(Entity* en, Vector2 start_pos, Vector2 fire_dir, float32 base_dmg) {
	en->arch = ARCH_arrow;
	en->sprite_id = SPRITE_arrow;
	en->renderable = true;
	en->is_projectile = true;
	en->projectile_range = 200;
	en->projectile_fire_dir = fire_dir;
	en->projectile_start_pos = start_pos;
	en->projectile_speed = 5;
	en->pos = start_pos;
	en->damage = base_dmg + 5;
	en->hitbox[0] = v2(0, 0);
	en->hitbox[1] = v2(10, 0);
	en->hitbox[2] = v2(0, 3);
	en->hitbox[3] = v2(10, 3);
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
	sprite_load(STR("assets/starterbow.png"), SPRITE_starterbow);
	sprite_load(STR("assets/arrow.png"), SPRITE_arrow);

	Entity* player = entity_create();
	player_setup(player);
	player->primary_weapon = ARCH_startersword;
	player->secondary_weapon = ARCH_starterbow;

	Entity* player_weapon = entity_create();
	weapon_setup(player_weapon, player->primary_weapon);

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
		player_weapon->weapon_owner_pos = player->pos;
		if (move_axis.x != 0 || move_axis.y != 0) {
			player_weapon->weapon_dir = move_axis;
		}

		// weapon swapping
		if (is_key_just_pressed('X')) {
			swap(player->primary_weapon, player->secondary_weapon, Entity_Archetype);
			entity_destroy(player_weapon);
			player_weapon = entity_create();
			weapon_setup(player_weapon, player->primary_weapon);
			player_weapon->pos = player->pos;
		}

		// :generic simulation (i.e. not a specific boss or player)
		for (Entity* en = 0; entity_increment(&en);) {
			if (en->is_animate && en->health <= 0) {
				en->alive = false;
			}

			if (en->is_weapon) {
				en->weapon_cooldown_secs -= 1.0 * delta_time; // decreases by 1 every second
				if (is_key_just_pressed('Z') && en->weapon_cooldown_secs <= 0) {
					en->weapon_cooldown_secs = 1 / (en->weapon_speed);
					switch (en->weapon_class) {
						case WEAPON_melee: {
							Vector2* hitbox = entity_resolve_hitbox(en);
							for (Entity* other_en = 0; entity_increment(&other_en);) {
								Vector2* other_hitbox = entity_resolve_hitbox(other_en);

								if (gjk(hitbox, 4, other_hitbox, 4)) {
									other_en->health -= en->damage;
								}
							}
						} break;

						case WEAPON_ranged: {
							// TODO add other ammo, also add an actual limit once inventory is done
							Entity* arrow = entity_create();
							arrow_setup(arrow, en->pos, en->weapon_dir, en->damage);
						} break;

						default: break;
					}
				}
			}

			if (en->is_projectile) {
				if (en->pos.x < en->projectile_start_pos.x + en->projectile_range &&
					en->pos.y < en->projectile_start_pos.y + en->projectile_range) {
					Vector2* hitbox = entity_resolve_hitbox(en);
					for (Entity* other_en = 0; entity_increment(&other_en);) {
						Vector2* other_hitbox = entity_resolve_hitbox(other_en);

						if (other_en->is_animate && gjk(hitbox, 4, other_hitbox, 4)) {
							other_en->health -= en->damage;
							entity_destroy(en);
						}
					}

					en->pos = v2_add(en->pos, v2_mulf(en->projectile_fire_dir, en->projectile_speed * 50.0 * delta_time));
				} else {
					entity_destroy(en);
				}
			}
		}

		// :rendering
		// TODO add a layer system, so that you place render instructions anywhere
		for (Entity* en = 0; entity_increment(&en);) {
			if (en->alive && en->renderable) {
				Sprite* sprite = sprite_get(en->sprite_id);
				Matrix4 xform = m4_scalar(1.0);

				if (en->is_projectile) {
					// face where the projectile is going
					xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
					xform = m4_rotate_z(xform, atan2(en->projectile_fire_dir.x, en->projectile_fire_dir.y) - 90 * RAD_PER_DEG);
				} else if (en->is_weapon) {
					// move the weapon away from its owner
					Vector2 dist_from_owner = v2_mulf(sprite->size, 1.25f);
					dist_from_owner.x = max(20, dist_from_owner.x);
					dist_from_owner.y = max(20, dist_from_owner.y);

					Vector2 target_pos = v2_add(en->weapon_owner_pos, v2_mul(en->weapon_dir, dist_from_owner));
					animate_to_target_v2(&(en->pos), target_pos, delta_time, 30.0f);

					float32 target_rads = atan2(en->weapon_dir.x, en->weapon_dir.y) - 45 * RAD_PER_DEG;
					animate_to_target_f32(&(en->weapon_rads), target_rads, delta_time, 15.0f);

					xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
					xform = m4_rotate_z(xform, en->weapon_rads);
				} else {					
					xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
				}

				xform = m4_translate(xform, v3(sprite->size.x * -0.5, sprite->size.y * -0.5, 0));
				draw_image_xform(sprite->image, xform, sprite->size, COLOR_WHITE);
			}
		}
		
		os_update(); 
		gfx_update();

		reset_temporary_storage();
	}

	return 0;
}