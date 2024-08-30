void weapon_setup(Entity* en, Entity_Archetype weapon_type) {
	en->arch = weapon_type;
	en->renderable = true;
	en->is_weapon = true;
	switch (weapon_type) {
		case ARCH_startersword: {
			en->weapon_class = WEAPON_melee;
			en->damage = 10;
			en->hitbox[0] = v2(0, 0);
			en->hitbox[1] = v2(0, 13);
			en->hitbox[2] = v2(13, 0);
			en->hitbox[3] = v2(13, 13);
			en->weapon_speed = 3;
		} break;

		case ARCH_starterbow: {
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
	en->renderable = true;
	en->is_projectile = true;
	en->is_animate = false; // idk why this was on in the first place?
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