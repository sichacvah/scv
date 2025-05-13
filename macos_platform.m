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
#include <Cocoa/Cocoa.h>
#include <AppKit/AppKit.h>
#include <objc/runtime.h>

#define SCV_PAGE_SIZE 16384
#include "scv.h"
#include "scv_geom.h"
#include "scv_linalg.h"
#include "macos_syscalls.h"


#define global_variable static
#define internal static

global_variable bool running = true;
global_variable f32 globalRenderWidth = 1024;
global_variable f32 globalRenderHeight = 768;

@interface SCVMainWindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation SCVMainWindowDelegate

- (void)windowWillClose:(id)sender {
  running = false;
}

@end

void
FillPixel(SCVBitmap bitmap, i32 x, i32 y) {
  i32 indx = (i32)bitmap.pitch * y + x * 4;
  byte *bytes = (byte *)bitmap.data;
  bytes[0 + indx] = 255;
  bytes[1 + indx] = 0;
  bytes[2 + indx] = 0;
  bytes[3 + indx] = 255;
}

void
DrawPolygon(SCVBitmap bitmap, i32 corners, SCVPoint* points)
{
  i32 nodes, nodex[256], pixelX, pixelY, i, j, swap;
  for (pixelY = 0; pixelY < (i32)bitmap.height; pixelY++) {
    nodes = 0;
    j = corners - 1;
    for (i = 0; i < corners; i++) {
      if ((points[i].y < (f64)pixelY && points[j].y >= (f64)pixelY) ||
          (points[j].y < (f64)pixelY && points[i].y >= (f64)pixelY)) {
        nodex[nodes++] = (i32)(points[i].x+((f32)pixelY - points[i].y) / (points[j].y - points[i].y) * (points[j].x - points[i].x));
      }
      j = i;
    } 

    i = 0;
    while (i < nodes-1) {
      if (nodex[i] > nodex[i+1]) {
        swap = nodex[i];
        nodex[i] = nodex[i+1];
        nodex[i+1] = swap;
        if (i) {
          i--;
        }
      } else {
        i++;
      }
    }


    for (i=0; i<nodes; i+=2) {
      if (nodex[i] >= (i32)bitmap.width) {
        break;
      }
      if (nodex[i + 1] < 0) {
        continue;
      }

      if (nodex[i] < 0) {
        nodex[i] = 0;
      }

      if (nodex[i+1] > (i32)bitmap.width) {
        nodex[i + 1] = (i32)bitmap.width;
      }
      for (pixelX = nodex[i]; pixelX<nodex[i+1]; pixelX++) {
        FillPixel(bitmap, pixelX, pixelY);
      }
    }
  }
}

void
DrawRect(SCVBitmap bitmap, u32 posx, u32 posy, u32 width, u32 height, SCVColor color)
{
  scvAssert(posx < bitmap.width);
  scvAssert(posy < bitmap.height);
  scvAssert(posx + width <= bitmap.width);
  scvAssert(posy + height <= bitmap.height);
  u8 *bytes = (u8 *)bitmap.data;

  for (u32 y = posy; y < posy + height; ++y) {
    u32 pos = y * bitmap.pitch;
    for (u32 x = posx * 4; x < (posx + width) * 4; x += 4) {
      u32 indx = x + pos;
      bytes[indx + 0] = color.r;
      bytes[indx + 1] = color.g;
      bytes[indx + 2] = color.b;
      bytes[indx + 3] = color.a;
    }
  }
}

int
main(int argc, char **argv)
{
  unused(argc);
  unused(argv);

  SCVMainWindowDelegate *mainWindowDelegate = [[SCVMainWindowDelegate alloc] init];

  NSRect screenRect = [[NSScreen mainScreen] frame];

  NSRect initialFrame = NSMakeRect(
    (screenRect.size.width - globalRenderWidth) * 0.5,
    (screenRect.size.height - globalRenderHeight) * 0.5,
    globalRenderWidth,
    globalRenderHeight
  );

  NSWindow *window = [[NSWindow alloc] initWithContentRect:initialFrame
                                        styleMask:NSWindowStyleMaskTitled |
                                                  NSWindowStyleMaskClosable |
                                                  NSWindowStyleMaskMiniaturizable |
                                                  NSWindowStyleMaskResizable
                                          backing:NSBackingStoreBuffered
                                            defer:NO];

  [window setBackgroundColor: NSColor.blackColor];
  [window setTitle: @"Renderer playground"];
  [window makeKeyAndOrderFront: nil];
  [window setDelegate: mainWindowDelegate];
  window.contentView.wantsLayer = YES;

  int bitmapWidth = window.contentView.bounds.size.width;
  int bitmapHeight = window.contentView.bounds.size.height;
  int bytesPerPixel = 4;
  int pitch = bitmapWidth * bytesPerPixel;
  int size = pitch*bitmapHeight;

  u8* buffer = scvMmap(nil, scvSizeRoundUp(size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0, nil);
  memset(buffer, 255, size);

  SCVBitmap bitmap = (SCVBitmap){
    .data   = (void *)buffer,
    .width  = bitmapWidth,
    .height = bitmapHeight,
    .pitch  = pitch
  };

  SCVPoint points[3];
  points[0].x = 10.0f;
  points[0].y = 10.0f;

  points[1].x = 310.0f;
  points[1].y = 10.0f;
  
  points[2].x = 10.0f;
  points[2].y = 610.0f;

  DrawPolygon(bitmap, 3, points);

 /*
  int cw = bitmapWidth - 40;
  int ch = bitmapHeight - 40;

  int offx = 20;
  int offy = 20;
  int rw = cw / 3;
  int rh = ch / 2;

  DrawRect(bitmap, 0, 0, bitmapWidth, bitmapHeight, (SCVColor) {
    .r = 25,
    .g = 23,
    .b = 36,
    .a = 255
  });

  DrawRect(bitmap, offx, offy, rw, rh, (SCVColor) {
    .r = 196,
    .g = 167,
    .b = 231,
    .a = 255
  });


  DrawRect(bitmap, offx + rw, offy, rw, rh, (SCVColor) {
    .r = 49,
    .g = 116,
    .b = 143,
    .a = 255
  });

  DrawRect(bitmap, offx + rw * 2, offy, rw, rh, (SCVColor) {
    .r = 235,
    .g = 111,
    .b = 146,
    .a = 255
  });

  DrawRect(bitmap, offx, offy + rh, rw, rh, (SCVColor) {
    .r = 235,
    .g = 188,
    .b = 186,
    .a = 255
  });


  DrawRect(bitmap, offx + rw, offy + rh, rw, rh, (SCVColor) {
    .r = 246,
    .g = 193,
    .b = 119,
    .a = 255
  });

  DrawRect(bitmap, offx + rw * 2, offy + rh, rw, rh, (SCVColor) {
    .r = 156,
    .g = 207,
    .b = 216,
    .a = 255
  });
  */

  while (running) {

    @autoreleasepool {
      NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes: &buffer
        pixelsWide:bitmapWidth 
        pixelsHigh:bitmapHeight
        bitsPerSample:8
        samplesPerPixel:4
        hasAlpha:YES
        isPlanar:NO
        colorSpaceName:NSDeviceRGBColorSpace
        bytesPerRow:pitch
        bitsPerPixel:bytesPerPixel*8] autorelease];

      NSSize imageSize = NSMakeSize(bitmapWidth, bitmapHeight);
      NSImage *image = [[[NSImage alloc] initWithSize: imageSize] autorelease];
      [image addRepresentation:rep];
      window.contentView.layer.contents = image;
    }

    NSEvent *Event;
    do {
      Event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                 untilDate: nil
                                    inMode: NSDefaultRunLoopMode
                                   dequeue: YES];
      
      switch ([Event type]) {
          default:
              [NSApp sendEvent: Event];
      }
    } while (Event != nil);
  }

  scvPrintCString("Finish running");
}

