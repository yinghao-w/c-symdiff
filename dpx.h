#ifndef DPX_KT
#ifndef DPX_VT
#ifndef DPX_PFX
typedef int DPX_DEBUG_TYPE;
#define DPX_KT DPX_DEBUG_TYPE
#define DPX_VT DPX_DEBUG_TYPE
#define DPX_PFX voidchar
#endif
#endif
#endif

#ifdef DPX_KT
#ifdef DPX_VT
#ifdef DPX_PFX

#ifndef DPX_STRUCT_PFX
#define DPX_STRUCT_PFX DPX_PFX
#endif

#include <stdlib.h>
#include <string.h>

#define DPX_CONCAT_2(A, B) A##B
#define DPX_CONCAT(A, B) DPX_CONCAT_2(A, B)
#define DPXMap DPX_CONCAT(DPX_STRUCT_PFX, Map)

typedef struct DPXMap DPXMap;

static DPXMap *DPX_CONCAT(DPX_PFX, _create)(size_t cap);

static DPX_VT DPX_CONCAT(DPX_PFX, _get)(DPX_KT key, const DPXMap *map);

static DPX_VT *DPX_CONCAT(DPX_PFX, _addr)(DPX_KT key, const DPXMap *map);

static void DPX_CONCAT(DPX_PFX, _add)(DPX_KT key, DPX_VT value, DPXMap *map);

static int DPX_CONCAT(DPX_PFX, _is_in)(DPX_KT key, const DPXMap *map);

static size_t DPX_CONCAT(DPX_PFX, _size)(const DPXMap *map);

static void DPX_CONCAT(DPX_PFX, _destroy)(DPXMap *map);

typedef struct {
  DPX_KT key;
  DPX_VT value;
} DPX_CONCAT(DPX_STRUCT_PFX, Bucket);

struct DPX_CONCAT(DPX_STRUCT_PFX, Map) {
  size_t size;
  size_t cap;
  DPX_CONCAT(DPX_STRUCT_PFX, Bucket) * data;
};

static DPXMap *DPX_CONCAT(DPX_PFX, _create)(size_t cap) {
  DPXMap *p = malloc(sizeof(*p));
  p->size = 0;
  p->cap = cap;
  p->data = malloc(cap * sizeof(*p->data));
  return p;
}

static void DPX_CONCAT(DPX_PFX, _destroy)(DPXMap *map) {
  free(map->data);
  free(map);
}

static size_t DPX_CONCAT(DPX_PFX, _size)(const DPXMap *map) {
  return map->size;
}

static DPX_VT DPX_CONCAT(DPX_PFX, _get)(DPX_KT key, const DPXMap *map) {
  for (size_t i = 0; i < DPX_CONCAT(DPX_PFX, _size)(map); i++) {
    if (map->data[i].key == key) {
      return map->data[i].value;
    }
  }
  DPX_VT temp;
  memset(&temp, 0, sizeof(DPX_VT));
  return temp;
}

static DPX_VT *DPX_CONCAT(DPX_PFX, _addr)(DPX_KT key, const DPXMap *map) {
  for (size_t i = 0; i < DPX_CONCAT(DPX_PFX, _size)(map); i++) {
    if (map->data[i].key == key) {
      return &map->data[i].value;
    }
  }
  return NULL;
}

void DPX_CONCAT(DPX_PFX, _add)(DPX_KT key, DPX_VT value, DPXMap *map) {
  for (size_t i = 0; i < DPX_CONCAT(DPX_PFX, _size)(map); i++) {
    if (map->data[i].key == key) {
      map->data[i].value = value;
      return;
    }
  }
  if (map->size >= map->cap) {
    map->data = realloc(map->data, 2 * map->cap * sizeof(*map->data));
    map->cap *= 2;
  }
  map->data[map->size++] =
      (DPX_CONCAT(DPX_STRUCT_PFX, Bucket)){.key = key, .value = value};
}

static int DPX_CONCAT(DPX_PFX, _is_in)(DPX_KT key, const DPXMap *map) {
  for (size_t i = 0; i < DPX_CONCAT(DPX_PFX, _size)(map); i++) {
    if (map->data[i].key == key) {
      return 1;
    }
  }
  return 0;
}

#undef DPX_CONCAT
#undef DPX_CONCAT_2

#undef DPX_KT
#undef DPX_VT
#undef DPX_PFX
#undef DPX_STRUCT_PFX

#endif
#endif
#endif
