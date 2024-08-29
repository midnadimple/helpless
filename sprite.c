typedef struct Sprite {
	Gfx_Image* image;
	Vector2 size;
} Sprite;

typedef enum Sprite_ID {
    SPRITE_nil,
	SPRITE_player,
	SPRITE_spider,
	SPRITE_mutant,
	SPRITE_tree,
	SPRITE_startersword,
	SPRITE_starterbow,
    SPRITE_arrow,
	SPRITE_MAX, 
} Sprite_ID;
Sprite sprites[SPRITE_MAX];

void sprite_load(string path, Sprite_ID id) {
	Gfx_Image *img = load_image_from_disk(path, get_heap_allocator());
	assert(img, "Image not found D:");
	sprites[id].image = img;
	sprites[id].size = v2((float32)img->width, (float32)img->height);
}

Sprite* sprite_get(Sprite_ID id) {
	if (id >= 0 && id < SPRITE_MAX) {
		return &sprites[id];
	}
	return &sprites[0];
}

Sprite_ID sprite_get_id_from_arch(Entity_Archetype arch) {
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