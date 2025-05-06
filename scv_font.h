#ifndef SCV_FONT
#define SCV_FONT

u16
scvGetU16FromStart(SCVSlice bytes)
{
  u8 *base = (u8 *)bytes.base;
  scvAssert(bytes.len > 2);

  return ((u16)base[0] << 8 | (u16)base[1]);
}

i16
scvGetI16FromStart(SCVSlice bytes)
{
  return (i16)(scvGetU16FromStart(bytes));
}

f32
scvGetF2_14FromStart(SCVSlice bytes)
{
  return ((f32)scvGetI16FromStart(bytes) * (1.0f / (f32)(1 << 14)));
}

u32
scvGetU32FromStart(SCVSlice bytes)
{
  u8 *base = (u8 *)bytes.base;
  scvAssert(bytes.len > 4);

  return (((u32)base[0] << 24) | ((u32)base[1] << 16) | ((u32)base[2] << 8) | (u32)base[3]);
}

#define scvGetU32(bytes, off) scvGetU32FromStart(scvSliceLeft((bytes), (off)))
#define scvGetU16(bytes, off) scvGetU16FromStart(scvSliceLeft((bytes), (off)))
#define scvGetI16(bytes, off) scvGetI16FromStart(scvSliceLeft((bytes), (off)))

void
scvFontParse(SCVSlice bytes, SCVError *error)
{
  scvAssert(bytes.base);
  if (bytes.len < 12) {
    scvErrorSet(error, "Invalid font content", SCV_FONT_INVALID);
    return;
  }

  u32 version = scvGetU32FromStart(bytes); 
  u16 numTables = scvGetU16(bytes, 4);

  scvPrintU64((u64)10);
  scvPrint("version = "); 
  scvPrintU64((u64)version);
  scvPrint("num table = ");
  scvPrintU64((u64)numTables);
}



#endif
