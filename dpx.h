#ifndef DPX_KT
#ifndef DPX_VT
#define DPX_KT void *
#define DPX_VT char *
#endif
#endif

#include <stdlib.h>
#include <string.h>

typedef struct DPX_map DPX_map;

static DPX_map *DPX_create(size_t cap);

static DPX_VT DPX_get(DPX_KT key, const DPX_map *map);

static DPX_VT *DPX_addr(DPX_KT key, const DPX_map *map);

static void DPX_add(DPX_KT key, DPX_VT value, DPX_map *map);

static int DPX_is_in(DPX_KT key, const DPX_map *map);

static size_t DPX_size(const DPX_map *map);

static void DPX_destroy(DPX_map *map);

typedef struct {
	DPX_KT key;
	DPX_VT value;
} DPX_bucket;

struct DPX_map {
	size_t size;
	size_t cap;
	DPX_bucket *data;
};

static DPX_map *DPX_create(size_t cap) {
	DPX_map *p = malloc(sizeof(*p));
	p->size = 0;
	p->cap = cap;
	p->data = malloc(cap * sizeof(*p->data));
	return p;
}

static void DPX_destroy(DPX_map *map) {
	free(map->data);
	free(map);
}

static size_t DPX_size(const DPX_map *map) {
	return map->size;
}

static DPX_VT DPX_get(DPX_KT key, const DPX_map *map) {
	for (size_t i = 0; i < DPX_size(map); i++) {
		if (map->data[i].key == key) {
			return map->data[i].value;
		}
	}
	DPX_VT temp;
	memset(&temp, 0, sizeof(DPX_VT));
	return temp;
}

static DPX_VT *DPX_addr(DPX_KT key, const DPX_map *map) {
	for (size_t i = 0; i < DPX_size(map); i++) {
		if (map->data[i].key == key) {
			return &map->data[i].value;
		}
	}
	return NULL;
}

void DPX_add(DPX_KT key, DPX_VT value, DPX_map *map) {
  for (size_t i = 0; i < DPX_size(map); i++) {
    if (map->data[i].key == key) {
		map->data[i].value = value;
		return;
    }
  }
  if (map->size >= map->cap) {
	  map->data = realloc(map->data, 2 * map->cap * sizeof(*map->data));
	  map->cap *= 2;
  }
  map->data[map->size++] = (DPX_bucket) {.key = key, .value = value};

}

static int DPX_is_in(DPX_KT key, const DPX_map *map) {
  for (size_t i = 0; i < DPX_size(map); i++) {
    if (map->data[i].key == key) {
		return 1;
    }
  }
  return 0;
}
