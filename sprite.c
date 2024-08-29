typedef struct Sprite {
	Gfx_Image* image;
	Vector2 size;
} Sprite;

typedef enum Sprite_ID {
	SPRITE_player,
	SPRITE_spider,
	SPRITE_mutant,
	SPRITE_tree,
	SPRITE_startersword,
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