#ifndef SCV_BITMAP
#define SCV_BITMAP

#define BI_ALPHABITFIELDS 108
#define BMP_FILE_HEADER   14

#define BMP_HEADER_SIZE (BI_ALPHABITFIELDS + BMP_FILE_HEADER)

#define BI_RGB 0
#define BI_BITFIELDS 3

u64
scvWriteU16ToSlice(SCVSlice slice, u16 i)
{

  u16 *base = (u16 *)slice.base;
  *base = isLittleEndian() ? i : scvSwapU16Endians(i);
  return 2;
}

u64
scvWriteI32ToSlice(SCVSlice slice, int i)
{
  int *base = (int *)slice.base;
  *base = isLittleEndian() ? i : scvSwapI32Endians(i);
  return 4;
}

typedef struct SCVBMP SCVBMP;
struct SCVBMP {
  SCVSlice bytes;
};

u32
scvBMPWidth(SCVBMP *bmp)
{
  SCVSlice bytes = bmp->bytes;
  u32 width = 0;
  u64 offset = BMP_FILE_HEADER + 4;
  byte *buf = (byte *)scvSliceLeft(bytes, offset).base;
  width = ((u32)(buf[3]) << 24) | ((u32)(buf[2]) << 16) | ((u32)(buf[1]) << 8) | ((u32)(buf[0]) << 0);
  return width;
}

u32
scvBMPHeight(SCVBMP *bmp)
{
  SCVSlice bytes = bmp->bytes;
  u32 height = 0;
  u64 offset = BMP_FILE_HEADER + 4 + 4;
  byte *buf = (byte *)scvSliceLeft(bytes, offset).base;
  height = ((u32)(buf[3]) << 24) | ((u32)(buf[2]) << 16) | ((u32)(buf[1]) << 8) | ((u32)(buf[0]) << 0);
  return height;
}

SCVSlice
scvBMPImageSlice(SCVBMP *bmp)
{
  SCVSlice bytes = bmp->bytes;
  return scvSliceLeft(bytes, BMP_HEADER_SIZE);
}

SCVSlice
scvAllocBMP(u32 width, u32 height, SCVError *error)
{
  u64 pitch = width * 4;
  u64 imgSize = height * pitch;
  u64 size = imgSize + BMP_HEADER_SIZE;
  byte *base = (byte*)scvMmap(nil, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0, error);
  memset(base, 0, size);
  return scvUnsafeSlice(base, size); 
}

void
scvColorPixel(SCVBMP *bmp, u32 x, u32 y, SCVColor color)
{
  SCVSlice imgarr = scvBMPImageSlice(bmp);
  u32 width = scvBMPWidth(bmp);
  //u32 height = scvBMPHeight(bmp);

  u32 pos = y * width * 4 + x * 4;
  byte *buf = (byte *)imgarr.base;
  //buf = buf + pos;
  buf[pos + 0] = color.a;
  buf[pos + 1] = color.r;
  buf[pos + 2] = color.g;
  buf[pos + 3] = color.b;
}

SCVBMP
scvCreateBMP(SCVSlice bytes, u32 width, u32 height)
{
  SCVBMP bmp;
  u64 pitch = width * 4;
  u64 imgSize = height * pitch;
  u64 size = imgSize + BMP_HEADER_SIZE;
  u64 offset = 0;
  byte *base = (byte *)bytes.base;

  // write header
  base[offset++] = 'B';
  base[offset++] = 'M';
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), size);

  offset += 4;
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), BMP_HEADER_SIZE);

  // write DIB header
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), BI_ALPHABITFIELDS);
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), width);
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), height);

  offset += scvWriteU16ToSlice(scvSliceLeft(bytes, offset), 1);
  offset += scvWriteU16ToSlice(scvSliceLeft(bytes, offset), 32);
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), BI_BITFIELDS); // BI_BITFIELDS 
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), imgSize);  
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 2835); // 72 DPI pritn resolution
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 2835); // 72 DPI pritn resolution
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 0); // numbers of colors in palette
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 0); // 0 means all colors are important
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 0x0000FF00); // red channel mask
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 0x00FF0000); // green channel mask
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 0xFF000000); // blue channel mask
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 0x000000FF); // alpha channel mask
  offset += scvWriteI32ToSlice(scvSliceLeft(bytes, offset), 0x206E6957); // 'Win ' in hex
  offset += 36; // CIEXYZTRIPLE Color Space endpoints, unused so just skip
  
/*
  SCVSlice imageArr = scvSliceLeft(bytes, offset);

  byte *arr = (byte *)imageArr.base;
  for (u64 y = 0; y < height; y++) {
    u64 p = pitch * y;
    for (u64 x = 0; x < width * 4; x += 4) {
      u64 pos = p + x;
      arr[pos + 0] = 0xFF; // alpha
      arr[pos + 1] = 0x00; // red
      arr[pos + 2] = 0x00; // green
      arr[pos + 3] = 0xFF; // blue
    }
  }
*/
  bmp.bytes = bytes;
  return bmp;
}





#endif
