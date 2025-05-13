#ifndef SCV_WEBSOCKETS
#define SCV_WEBSOCKETS

#define MAX_REQUEST_LINE  128
#define MAX_HEADERS_COUNT 32
#define MAX_HEADER_SIZE 64

typedef struct SCVHTTPHeader SCVHTTPHeader;
struct SCVHTTPHeader {
  SCVString key;
  SCVString value;
};

enum SCV_HTTP_METHOD {
  SCV_HTTP_UNKNOWN = 0,
  SCV_HTTP_GET,
  SCV_HTTP_POST,
  SCV_HTTP_PUT,
  SCV_HTTP_PATCH,
  SCV_HTTP_DELETE
};
typedef enum SCV_HTTP_METHOD SCV_HTTP_METHOD;

typedef struct SCVHTTPRequest SCVHTTPRequest;
struct SCVHTTPRequest {
  SCVHTTPHeader headers[MAX_HEADERS_COUNT];
  SCVSlice bytes;
  SCVSlice body; 
  SCVString uri;
  SCVString httpVer;
  SCV_HTTP_METHOD method;
  u32 headersCount;
  bool isValid;
};

u32
scvHTTPOffsetByMethod(SCV_HTTP_METHOD method)
{
  switch (method) {
    case SCV_HTTP_PUT:
    case SCV_HTTP_GET:
      return 4;
    case SCV_HTTP_POST:
      return 5;
    case SCV_HTTP_PATCH:
      return 6;
    case SCV_HTTP_DELETE:
      return 7;
    default:
      return 0;    
  }
}

SCV_HTTP_METHOD
scvHTTPMethod(SCVSlice bytes, u32 *offset)
{
  scvAssert(bytes.len > 7);
  scvAssert(offset);
  byte *base = (byte *)bytes.base;
  if (base[0] == 'G' && base[1] == 'E' && base[2] == 'T' && base[3] == ' ') {
    *offset = 4;
    return SCV_HTTP_GET;
  } else if (base[0] == 'P' && base[1] == 'O' && base[2] == 'S' && base[3] == 'T' && base[4] == ' ') {
    *offset = 5;
    return SCV_HTTP_POST;
  } else if (base[0] == 'P' && base[1] == 'U' && base[2] == 'T' && base[3] == ' ') {
    *offset = 4;
    return SCV_HTTP_PUT;
  } else if (base[0] == 'P' && base[1] == 'A' && base[2] == 'T' && base[3] == 'C' && base[4] == 'H' && base[5] == ' ') {
    *offset = 6;
    return SCV_HTTP_PATCH;
  } else if (base[0] == 'D' && base[1] == 'E' && base[2] == 'L' && base[3] == 'E' && base[4] == 'T' && base[5] == 'E' && base[6] == ' ') {
    *offset = 7;
    return SCV_HTTP_DELETE;
  }
  return SCV_HTTP_UNKNOWN;
}

i32
scvFindSpace(SCVSlice bytes, i32 m)
{
  i32 n;
  byte *base = (byte *)bytes.base;
  i32 count = scvMin(m, (i32)bytes.len - 1);
  for (i32 i = 0; i < count; ++i) {
    if (base[i] == ' ') {
      n = i + 1;
      return n;
    }
  }

  return -1;
}

i32
scvFindEndLine(SCVSlice bytes, i32 m)
{
  i32 n = -1;
  byte *base = (byte *)bytes.base;
  i32 count = scvMin((i32)bytes.len - 1, m);
  for (i32 i = 0; i < count - 1; i += 2) {
    if (base[i] == '\r' && base[i + 1] == '\n') {
      return i + 2;
    }
  }

  return n;
}

i32
scvHTTPFindHeaderSeparator(SCVSlice bytes, i32 m)
{
  i32 n = -1;
  byte *base = (byte *)bytes.base;
  i32 count = scvMin((i32)(bytes.len - 1), m);
  for (i32 i = 0; i < count; i += 2) {
    if (base[i] == ':' && base[i + 1] == ' ') {
      return i + 1;
    }
  }
  return n;
}

u32
scvHTTPParseMethod(SCVHTTPRequest *req)
{
  u32 offset = 0;
  scvAssert(req); 
  req->method = scvHTTPMethod(req->bytes, &offset);
  if (req->method == SCV_HTTP_UNKNOWN) {
    req->isValid = false;
  }

  return offset;
}

u32
scvHTTPExtractURI(SCVHTTPRequest *req, u32 offset)
{
  i32 n = 0;
  scvAssert(req);
  scvAssert(req->bytes.len > offset);
  SCVSlice bytes = scvSliceLeft(req->bytes, offset);
  n = scvFindSpace(bytes, MAX_REQUEST_LINE - offset);
  if (n < 1) {
    req->isValid = false;
    return 0;
  }
  req->uri = scvString(scvSliceRight(bytes, n - 1));
  return (u32)n;
}

u32
scvHTTPExtractVersion(SCVHTTPRequest *req, u32 offset)
{
  i32 n = 0;
  scvAssert(req);
  scvAssert(req->bytes.len > offset);
  SCVSlice bytes = scvSliceLeft(req->bytes, offset);
  n = scvFindEndLine(bytes, MAX_REQUEST_LINE - offset);
  if (n < 1) {
    req->isValid = false;
    return 0;
  }
  req->httpVer = scvString(scvSliceRight(bytes, n - 2));
  if (!scvIsStringsEquals(req->httpVer, scvUnsafeCString("HTTP/1.1"))) {
    req->isValid = false;
    return (u32)n;
  }
  return (u32)n; 
}

u32
scvHTTPExtractHeaderKey(SCVHTTPRequest *req, u32 offset, u32 hid)
{
  SCVHTTPHeader *header;
  i32 n = 0;
  scvAssert(req);
  scvAssert(req->bytes.len > offset);
  scvAssert(hid < MAX_HEADERS_COUNT);
  SCVSlice bytes = scvSliceLeft(req->bytes, offset);
  header = req->headers + hid;
  n = scvHTTPFindHeaderSeparator(bytes, MAX_HEADER_SIZE);
  if (n < 3 && n < MAX_HEADERS_COUNT - 4) {
    req->isValid = false;
    return 0;
  }
  header->key = scvString(scvSliceRight(bytes, n - 2));
  return n;
}


u32
scvHTTPExtractHeaderValue(SCVHTTPRequest *req, u32 offset, u32 hid)
{
  SCVHTTPHeader *header;
  i32 n = 0;
  scvAssert(req);
  scvAssert(req->bytes.len > offset);
  scvAssert(hid < MAX_HEADERS_COUNT);
  SCVSlice bytes = scvSliceLeft(req->bytes, offset);
  header = req->headers + hid;
  n = scvHTTPFindHeaderSeparator(bytes, MAX_HEADER_SIZE - header->key.len);
  if (n < 3) {
    req->isValid = false;
    return 0;
  }
  header->value = scvString(scvSliceRight(bytes, n - 2));
  return n;
}

void
scvHTTPParseRequest(SCVHTTPRequest *req, SCVSlice bytes) 
{
  scvAssert(req);
  memset(req, 0, sizeof(SCVHTTPRequest));
  req->isValid = true;
  u32 n = 0;
  req->bytes = bytes; 

  n = scvHTTPParseMethod(req);
  if (!req->isValid) {
    return;
  }
  n += scvHTTPExtractURI(req, n);
  if (!req->isValid) {
    return;
  }
  n += scvHTTPExtractVersion(req, n);
  if (!req->isValid) {
    return;
  }
  for (u32 i = 0; i < MAX_HEADERS_COUNT; i++) {
    n += scvHTTPExtractHeaderKey(req, n, i);
    if (!req->isValid) {
      return;
    }
    n += scvHTTPExtractHeaderValue(req, n, i);
    if (!req->isValid) {
      return;
    }
  }   
}

#endif
