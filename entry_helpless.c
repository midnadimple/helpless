#include "util.c"
#include "sprite.c"
#include "entity.c"
#include "gjk.c"
#include "weapon.c"
#include "player.c"

void spider_setup(Entity* en) {
	en->arch = ARCH_spider;
	en->renderable = true;
	en->is_animate = true;
	en->is_hostile = true;
	en->max_health = 30;
	en->enemy_speed = 5;
	en->health = en->max_health;
	en->damage = 10;
	en->pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
	en->hitbox[0] = v2(0, 0);
	en->hitbox[1] = v2(0, 7);
	en->hitbox[2] = v2(20, 0);
	en->hitbox[3] = v2(20, 7);
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
	memset(world, 0, sizeof(World));

	sprite_load(STR("assets/player.png"), SPRITE_player);
	sprite_load(STR("assets/spider.png"), SPRITE_spider);
	sprite_load(STR("assets/startersword.png"), SPRITE_startersword);
	sprite_load(STR("assets/starterbow.png"), SPRITE_starterbow);
	sprite_load(STR("assets/arrow.png"), SPRITE_arrow);

	Gfx_Font* font = load_font_from_disk(STR("assets/m3x6.ttf"), get_heap_allocator());
	assert(font, "Couldn't load font");

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

	float real_health_width = 0.0;

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

		if (is_key_just_pressed(KEY_F11)) {
			window.fullscreen = !window.fullscreen;
		}

		if (is_key_just_pressed(KEY_ESCAPE)) {
			window.should_close = true;
		}

		if (player->alive) {
			player_process_input(player, player_weapon, delta_time);
		} else {
			// TODO add game over state
			player_weapon->alive = false;
		}

		// :generic simulation (i.e. not a specific boss or player)
		for (Entity* en = 0; entity_increment(&en);) {
			if (en->is_animate && en->health <= 0) {
				en->alive = false;
			}

			if (en->is_hostile) {
				en->enemy_cooldown_secs -= 1.0 * delta_time;
				if (en->enemy_cooldown_secs <= 0) {
					Vector2* player_hitbox = entity_resolve_hitbox(player);
					Vector2* hitbox = entity_resolve_hitbox(en);
					if (gjk(hitbox, 4, player_hitbox, 4)) {
						entity_take_damage(player, en->damage);
						en->enemy_cooldown_secs = 4 / en->enemy_speed;
					}
				}
			}

			// TODO Add Armour (one piece set)

			// TODO Animate all this stuff
			
			// TODO Finish item pickup system
			// if (en->is_item && is_key_just_pressed('E')) {
			// 	Vector2* player_hitbox = entity_resolve_hitbox(player);
			// 	Vector2* item_hitbox = entity_resolve_hitbox(en);

			// 	if (gjk(player_hitbox, 4, item_hitbox, 4)) {
			// 		entity_destroy(en);
			// 	}
			// }

			// TODO Add inventory system using item_inv_size

			// TODO Add NPCs you can talk to (dialogue system)
			// TODO Add NPCs who sell you stuff (shop system)
			// TODO Add choices on what to do with stuff (decision tree system)

			// ONCE ALL TODOs are done, MVP prototype is done, and then you just spam content (enemy AI, bosses, world gen, items, decisions, NPCs)

			if (en->is_weapon) {
				en->weapon_cooldown_secs -= 1.0 * delta_time; // decreases by 1 every second
				if (is_key_down(MOUSE_BUTTON_LEFT) && en->weapon_cooldown_secs <= 0) {
					en->weapon_cooldown_secs = 1 / (en->weapon_speed);

					switch (en->weapon_class) {
						case WEAPON_melee: {
							Vector2* hitbox = entity_resolve_hitbox(en);
							for (Entity* other_en = 0; entity_increment(&other_en);) {
								Vector2* other_hitbox = entity_resolve_hitbox(other_en);

								if (gjk(hitbox, 4, other_hitbox, 4)) {
									entity_take_damage(other_en, en->damage);
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
				en->pos = v2_add(en->pos, v2_mulf(en->projectile_fire_dir, en->projectile_speed * 50.0 * delta_time));
				if (en->pos.x < en->projectile_start_pos.x + en->projectile_range &&
					en->pos.y < en->projectile_start_pos.y + en->projectile_range) {
					Vector2* hitbox = entity_resolve_hitbox(en);
					for (Entity* other_en = 0; entity_increment(&other_en);) {
						Vector2* other_hitbox = entity_resolve_hitbox(other_en);

						if (other_en->is_animate && gjk(hitbox, 4, other_hitbox, 4)) {
							entity_take_damage(other_en, en->damage);
							entity_destroy(en);
						}
					}

				} else {
					entity_destroy(en);
				}
			}
		}

		// :background world rendering (TBD when we have world data to load)
		{

		}

		// :entity rendering
		for (Entity* en = 0; entity_increment(&en);) {
			if (en->alive && en->renderable) {
				Sprite* sprite = sprite_get(entity_sprite_id_from_arch(en->arch));
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

		// :ui rendering
		{
			float width = window.width;
			float height = window.height;
			draw_frame.camera_xform = m4_scalar(1.0);
			draw_frame.projection = m4_make_orthographic_projection(0.0, width, 0.0, height, -1, 10);

			float weapon_slot_size = 16.0 * zoom;
			float weapon_slot_padding = 2.0 * zoom;

			// Primary Weapon Slot
			{
				Matrix4 xform = m4_scalar(1.0);
				xform = m4_translate(xform, v3(weapon_slot_padding, height - weapon_slot_size - weapon_slot_padding, 0.0));
				draw_rect_xform(xform, v2(weapon_slot_size, weapon_slot_size), hex_to_rgba(0x639bffff));

				Sprite* primary_weapon = sprite_get(entity_sprite_id_from_arch(player->primary_weapon));
				xform = m4_translate(xform, v3((weapon_slot_size - primary_weapon->size.x * zoom) / 2, (weapon_slot_size - primary_weapon->size.y * zoom) / 2, 0.0));
				xform = m4_scale(xform, v3(zoom, zoom, zoom));
				draw_image_xform(primary_weapon->image, xform, primary_weapon->size, COLOR_WHITE);
			}
			

			// Secondary Weapon Slot
			{
				Matrix4 xform = m4_scalar(1.0);
				xform = m4_translate(xform, v3(weapon_slot_padding, height - 2*weapon_slot_size - 2*weapon_slot_padding, 0.0));
				draw_rect_xform(xform, v2(weapon_slot_size, weapon_slot_size), hex_to_rgba(0x3f3f74ff));

				Sprite* secondary_weapon = sprite_get(entity_sprite_id_from_arch(player->secondary_weapon));
				xform = m4_translate(xform, v3((weapon_slot_size - secondary_weapon->size.x*zoom) / 2, (weapon_slot_size - secondary_weapon->size.y*zoom) / 2, 0.0));
				xform = m4_scale(xform, v3(zoom, zoom, zoom));
				draw_image_xform(secondary_weapon->image, xform, secondary_weapon->size, COLOR_WHITE);
			}

			float health_bar_width = (player->max_health / 2) * zoom;
			float health_bar_height = 8.0 * zoom;
			float health_bar_padding = 4.0 * zoom;
			Vector2 health_bar_pos = v2(weapon_slot_size + health_bar_padding, height - health_bar_height - weapon_slot_padding);

			// Health Bar
			{
				Matrix4 xform = m4_scalar(1.0);
				xform = m4_translate(xform, v3(health_bar_pos.x, health_bar_pos.y, 0.0));
				draw_rect_xform(xform, v2(health_bar_width, health_bar_height), hex_to_rgba(0x7b343aff));

				float target_width = (player->health / 2) * zoom;
				animate_to_target_f32(&real_health_width, target_width, delta_time, 20.0);
				draw_rect_xform(xform, v2(real_health_width, health_bar_height), hex_to_rgba(0xd95763ff));
			}
			
			// Health Text
			{
				string health_text;
				if (player->health >= player->max_health) {
					health_text = STR("full");
				} else if (player->health <= 0) {
					health_text = STR("dead");
				} else {
					health_text = tprint("%d", (int)player->health);
				}
				Gfx_Text_Metrics health_metrics = measure_text(font, health_text, 48, v2(1, 1));
				
				Vector2 health_text_pos = v2(health_bar_pos.x + health_bar_width/2, health_bar_pos.y + health_bar_height/2);
				Vector2 justified = v2_sub(health_text_pos, v2_divf(health_metrics.functional_size, 2));
				draw_text(font, health_text, 48, justified, v2(1, 1), COLOR_WHITE);
			}

			// TODO Add inventory menu

			// TODO Add dialogue box
		}
		
		os_update(); 
		gfx_update();

		reset_temporary_storage();
	}

	return 0;
}