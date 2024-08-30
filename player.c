void player_setup(Entity* en) {
	en->arch = ARCH_player;
	en->renderable = true;
	en->is_animate = true;
	en->max_health = 100;
	en->health = en->max_health;
    en->armour = ARMOUR_starter;
}

void player_process_input(Entity* player, Entity* player_weapon, float delta_time) {
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

    player->pos = v2_add(player->pos, v2_mulf(move_axis, 75 * delta_time));
    player_weapon->weapon_owner_pos = player->pos;
    player_weapon->weapon_dir = v2_normalize(get_mouse_pos_in_ndc());

    // weapon swapping
    if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {
        swap(player->primary_weapon, player->secondary_weapon, Entity_Archetype);
        entity_destroy(player_weapon);
        player_weapon = entity_create();
        weapon_setup(player_weapon, player->primary_weapon);
        player_weapon->pos = player->pos;
    }
}