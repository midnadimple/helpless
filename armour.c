typedef enum Armour {
    ARMOUR_none = 0,
    ARMOUR_starter = 1,
} Armour;

Sprite_ID armour_sprite_id(Armour armour) {
	switch (armour) {
		case ARMOUR_starter: return SPRITE_starterarmour; break;
		default: return SPRITE_nil;
	}
}

float armour_damage_multiplier(Armour armour) {
    switch (armour) {
        case ARMOUR_starter: return 0.8; break;
        default: return 1;
    }
}
