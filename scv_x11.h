#ifndef SCV_X11
#define SCV_X11

/*
 * .Xauthority file a binary file consisting of a
 * sequence of entries in the following format:
 * 
 * 2 bytes    Family value (second byte is as in protocol HOST)
 * 2 bytes    address length (always MSB first, BE)
 * A bytes    host address
 * 2 bytes    display "number" length always MSB first BE
 * S bytes    display "number" string
 * 2 bytes    name length (MSB first BE)
 * N bytes    authorization name 
 * 2 bytes    data length (MSB first BE)
 * D bytes    authorization data string
 */

static u16 SCVAuthEntryFamilyLocal = 1;
//SCVString SCVAuthEntryMagicCookie = scvUnsafeCString("MIT-MAGIC-COOKIE-1");

typedef struct SCVAuthToken SCVAuthToken;
struct SCVAuthToken {
  u8 data[16];
}; 

typedef struct SCVAuthEntry SCVAuthEntry;
struct SCVAuthEntry {
  u16 family;
  SCVString authName;
  SCVString authData;
  i32 valid;
};


SCVAuthToken
scvReadX11AuthEntry(SCVSlice buffer)
{
  SCVAuthEntry token = {0};
  u64 n = 0;

  if (buffer.len < sizeof(u16)) {
    return token;
  }
  n += sizeof(u16);

  token.family = *(u16 *)buffer.base;

  u16 addressLen = 0;
  buffer = scvSliceLeft(buffer, sizeof(u16));

  if (buffer.len < sizeof(u16)) {
    return token;
  }

  addressLen = *(u16 *)buffer.base;
  addressLen = scvHtons(addressLen);

  buffer = scvSliceLeft(buffer, sizeof(u16));
  if (buffer.len < addressLen) {
    return token;
  }

  SCVString address = scvString(
      scvSliceRight(buffer, (u64)addressLen)
  );
  scvPrint("address =");
  scvPrintString(address);

  buffer = scvSliceLeft(buffer, addressLen);
  if (buffer.len < sizeof(u16)) {
    return token;
  }

  u16 displayNumberLen = *(u16 *)buffer.base;
  displayNumberLen = scvHtons(displayNumberLen);

  if (buffer.len < displayNumberLen) {
    return token;
  }
  buffer = scvSliceLeft(buffer, sizeof(u16));

  SCVString displayNumber = scvString(
      scvSliceRight(buffer, (u64)displayNumberLen)
  ); 

  scvPrint("displayNumber =");
  scvPrintString(displayNumber);
  buffer = scvSliceLeft(buffer, (u64)displayNumberLen);
  if (buffer.len < sizeof(u16)) {
    return token;
  }

  u16 authNameLen = *(u16 *)buffer.base;
  buffer = scvSliceLeft(buffer, sizeof(u16));
  if (buffer.len < authNameLen) {
    return token;
  }

  token.authName = scvSliceRight(buffer, (u64)authNameLen);
  scvPrint("authName = ");
  scvPrintString(token.authName);
  buffer = scvSliceLeft(buffer, authNameLen);

  if (buffer.len < sizeof(u16)) {
    return token;
  }
  u16 authDataLen = *(u16 *)buffer.base;
  if (buffer.len < authDataLen) {
    return token;
  }

  token.authData = scvSliceRight(buffer, (u64)authDataLen);
  scvPrint("authData =");
  scvPrintString(token.authData);
  token.valid = 1;

  return token;
}


#endif
