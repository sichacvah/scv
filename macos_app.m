#include <sys/syscall.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/event.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <Cocoa/Cocoa.h>
#include <CoreVideo/CVDisplayLink.h>
#include <objc/runtime.h>
#include <OpenGL/gl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h"

#define SCV_PAGE_SIZE 16384

#include "scv.h"
#include "macos_syscalls.h"
#include "scv_geom.h"
#include "scv_linalg.h"
#include "scv_gl.h"
#include "app.h"

#define unused(a) (void)(a)


NSOpenGLContext *context;
bool isRunning = true;

NSApplication *NSApp;
NSWindow *win;

CVReturn
DisplayCallback (CVDisplayLinkRef displayLink, const CVTimeStamp *inNow,
                 const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn,
                 CVOptionFlags *flagsOut, void *displayLinkContext)
{

  unused (displayLink);
  unused (inNow);
  unused (inOutputTime);
  unused (flagsIn);
  unused (flagsOut);
  unused (displayLinkContext);

  assert (inOutputTime);

  // f64 fps = (f64)inOutputTime->rateScalar *
  // (f64)inOutputTime->videoTimeScale / (f64)inOutputTime->videoRefreshPeriod;
  // NSLog(@"FPS = %f", fps);

  [context makeCurrentContext];
  [context lock];
  AppUpdate(&GlobalContext);

  [context flushBuffer];
  [context unlock];
  return kCVReturnSuccess;
}

void FuncToSEL (char *className, char *registerName, void *function);

#define NSRelease(id) [id release]
#define FUNC_TO_SEL(className, function)                                      \
  FuncToSEL (className, #function ":", (void *)function)

void
FuncToSEL (char *className, char *registerName, void *function)
{
  Class selected;
  SCVString classNameStr = scvUnsafeCString (className);

  if (scvIsStringsEquals (classNameStr, scvUnsafeCString ("NSView")))
    {
      selected = objc_getClass ("ViewClass");
    }
  else if (scvIsStringsEquals (classNameStr, scvUnsafeCString ("NSWindow")))
    {
      selected = objc_getClass ("WindowClass");
    }
  else
    {
      selected = objc_getClass (className);
    }

  class_addMethod (selected, sel_registerName (registerName), (IMP)function,
                   0);
}

bool
windowShouldClose (void *c)
{
  (void)c;

  isRunning = false;

  return (true);
}

int
main (void)
{

  @autoreleasepool {

  NSApp = [NSApplication sharedApplication];
  NSLog(@"NSSCreen.backingScaleFactor = %f", [NSScreen mainScreen].backingScaleFactor);
  

  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  FUNC_TO_SEL("NSObject", windowShouldClose);

  NSOpenGLPixelFormatAttribute attribues[] = { NSOpenGLPFANoRecovery,
                                               NSOpenGLPFAAccelerated,
                                               NSOpenGLPFADoubleBuffer,
                                               NSOpenGLPFAColorSize,
                                               24,
                                               NSOpenGLPFAAlphaSize,
                                               8,
                                               NSOpenGLPFADepthSize,
                                               24,
                                               NSOpenGLPFAStencilSize,
                                               8,
                                               NSOpenGLPFAOpenGLProfile,
                                               NSOpenGLProfileVersion3_2Core,
                                               0 };

  NSRect rect = NSMakeRect(0, 0, 512, 512);
  win = [[NSWindow alloc]
      initWithContentRect:rect
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                          | NSWindowStyleMaskMiniaturizable
                          | NSWindowStyleMaskResizable
                  backing:NSBackingStoreBuffered
                    defer:NO];
  [win setTitle:@"Basic title"];

  CVDisplayLinkRef displayLink;
  NSOpenGLPixelFormat *format =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:attribues];
  NSOpenGLView *view =
      [[NSOpenGLView alloc] initWithFrame:rect
                              pixelFormat:format];
  [view prepareOpenGL];

  i32 swapInt = 1;
  context = view.openGLContext;
  [context setValues:&swapInt
        forParameter:NSOpenGLContextParameterSwapInterval];

  [context makeCurrentContext];
  AppInit(&GlobalContext, (SCVRect) {
      .origin = { rect.origin.x, rect.origin.y },
      .size   = { rect.size.width, rect.size.height }
  }, (f32)([NSScreen mainScreen].backingScaleFactor));

  CGDirectDisplayID displayID = CGMainDisplayID();
  CVDisplayLinkCreateWithCGDisplay(displayID, &displayLink);
  CVDisplayLinkSetOutputCallback (displayLink, DisplayCallback, win);
  CGLPixelFormatObj cglPixelFormat = [[view pixelFormat] CGLPixelFormatObj];
  CGLContextObj cglContext = [[view openGLContext] CGLContextObj];
  CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext (displayLink, cglContext,
                                                     cglPixelFormat);

  [win setContentView:(NSView *)view];
  [win setIsVisible:YES];
  [win makeMainWindow];
  [NSApp finishLaunching];

  CVDisplayLinkStart (displayLink);
  while (isRunning)
    {
      NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                          untilDate:[NSDate distantPast]
                                             inMode:NSDefaultRunLoopMode
                                            dequeue:YES];
      if (event.type == NSEventTypeKeyDown && event.keyCode == 12
          && (event.modifierFlags & NSEventModifierFlagCommand)
                 == NSEventModifierFlagCommand)
        {
          isRunning = false;
        }

      [NSApp sendEvent:event];
      [NSApp updateWindows];
    }

  CVDisplayLinkStop (displayLink);
  CVDisplayLinkRelease (displayLink);
  [view release];
  [NSApp terminate:nil];

  return 0;
  }
}

void 
scvLog(
    char *tag,
    enum SCVLogLevel level,
    u32 logitem,
    char* msg,
    u32 line,
    char* filename
) {
  SCVString loglvlstr;
  u8 linebuf[1024];
  u32 n = 0;

  SCVSlice s = scvUnsafeSlice(linebuf, sizeof(linebuf));

  switch (level) {
    case SCV_LOG_PANIC:
      loglvlstr = scvUnsafeCString("panic"); break;
    case SCV_LOG_ERROR:
      loglvlstr = scvUnsafeCString("error"); break;
    case SCV_LOG_WARN:
      loglvlstr = scvUnsafeCString("warn"); break;
    default:
      loglvlstr = scvUnsafeCString("info"); break;
  }

  n += scvSlicePutCString(scvSliceLeft(s, n), "[");
  n += scvSlicePutCString(scvSliceLeft(s, n), tag);
  n += scvSlicePutCString(scvSliceLeft(s, n), "]");

  n += scvSlicePutCString(scvSliceLeft(s, n), "[");
  n += scvSlicePutString(scvSliceLeft(s, n),  loglvlstr);
  n += scvSlicePutCString(scvSliceLeft(s, n), "]");

  if (logitem > 0) {
    n += scvSlicePutCString(scvSliceLeft(s, n), "[id:");
    n += scvSlicePutU64(scvSliceLeft(s, n), (u64)logitem);
    n += scvSlicePutCString(scvSliceLeft(s, n), "]");
  }

  if (filename) {
    // gcc/clang compiler error format
    n += scvSlicePutCString(scvSliceLeft(s, n), " ");
    n += scvSlicePutCString(scvSliceLeft(s, n), filename);
    n += scvSlicePutCString(scvSliceLeft(s, n), ":");
    n += scvSlicePutU64(scvSliceLeft(s, n), (u64)line);
    n += scvSlicePutCString(scvSliceLeft(s, n), ":0:");
  } else {
    n += scvSlicePutCString(scvSliceLeft(s, n), "[line:");
    n += scvSlicePutU64(scvSliceLeft(s, n), (u64)line);
    n += scvSlicePutCString(scvSliceLeft(s, n), "]");
  }

  if (msg) {
    n += scvSlicePutCString(scvSliceLeft(s, n), "\n\t");
    n += scvSlicePutCString(scvSliceLeft(s, n), msg);    
  }
  n += scvSlicePutCString(scvSliceLeft(s, n), "\n\n");

  if (level == SCV_LOG_PANIC) {
    n += scvSlicePutCString(scvSliceLeft(s, n), "ABORTING because of [panic]\n");
  }
  scvPrintString(scvString(scvSliceRight(s, n)));

  if (level == SCV_LOG_PANIC) {
    scvBreakpoint;
  }
}





