#ifndef SCV_GEOM
#define SCV_GEOM

// need to include scv.h before

typedef struct SCVPoint SCVPoint;
struct SCVPoint {
  f32 x;
  f32 y;
};

typedef struct SCVSize SCVSize;
struct SCVSize {
  f32 width;
  f32 height;
};

typedef struct SCVRect SCVRect;
struct SCVRect {
  SCVPoint origin;
  SCVSize  size;
};

typedef struct SCVColor SCVColor;
struct SCVColor {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
};


// TODO(sichirc): 4 bytes per pixel
typedef struct SCVBitmap SCVBitmap;
struct SCVBitmap {
  void  *data;
  u32   width;
  u32   height;
  u32   pitch;
};


#endif
