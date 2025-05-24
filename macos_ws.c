#include <stdbool.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <sys/mman.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#define PAGE_SIZE 16384
#include "scv.h"
#include "scv_font.h"
#include "scv_geom.h"
#include "scv_websockets.h"

__attribute__ (( __noinline__ )) SCVSyscallResult
scvSyscall(int trap, uptr a1, uptr a2, uptr a3)
{
  SCVSyscallResult r;

  __asm__ __volatile__(
    ".align 2\n" 
    "mov x16, %3\n"
    "mov x0, %4\n"
    "mov x1, %5\n"
    "mov x2, %6\n"
    "svc #0x80\n"
    "bcc 0f\n"
    "str x0, %2\n"
    "str x1, %1\n"
    "mov x1, #-1\n"
    "str x1, %0\n" 
    "b 1f\n"
    "0:\n"
    "str x0, %0\n"
    "str x1, %1\n"
    "mov x1, #0\n"
    "str x1, %2\n"
    "1:\n"
    :
    "=m" (r.r1), "=m" (r.r2), "=m" (r.err)
    :
    "r"((uptr)trap), "r"(a1), "r"(a2), "r"(a3)
    :
    "x0", "x1", "x2", "x3", "x16"
  );

  return r;
}

__attribute__ (( __noinline__ )) SCVSyscallResult
scvSyscall6(int trap, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5, uptr a6)
{
  SCVSyscallResult r;

  __asm__ __volatile__(
    ".align 2\n" 
    "mov x16, %x3\n"
    "mov x0, %4\n"
    "mov x1, %5\n"
    "mov x2, %6\n"
    "mov x3, %7\n"
    "mov x4, %8\n"
    "mov x5, %9\n"
    "svc #0x80\n"
    "bcc 0f\n"
    "str x0, %2\n"
    "str x1, %1\n"
    "mov x1, #-1\n"
    "str x1, %0\n" 
    "b 1f\n"
    "0:\n"
    "str x0, %0\n"
    "str x1, %1\n"
    "mov x1, #0\n"
    "str x1, %2\n"
    "1:\n"
    :
    "=m"(r.r1), "=m"(r.r2), "=m"(r.err)
    :
    "r"((uptr)trap), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6)
    :
    "x0", "x1", "x2", "x3", "x4", "x5", "x16"
  );

  return r;
}

/*
#define PF_INET 0x2
#define SOCK_STREAM 0x1
#define IPPROTO_IP 0x0
#define AF_INET  0x2
#define INADDR_ANY 0x0
*/
int 
main(void)
{
  i32 server, client;
  struct sockaddr_in hostaddr, clientaddr;
  SCVError error = {0};

  server = scvSocket(PF_INET, SOCK_STREAM, 0, &error);
  scvPrint("socketid = ");
  scvPrintU64((u64)server);
  if (error.tag) {
    goto error1;   
  }

  hostaddr.sin_family = AF_INET;
  hostaddr.sin_port   = scvHtons(1010);
  hostaddr.sin_addr.s_addr = INADDR_ANY;
  scvPrint("sinPort ");
  scvPrintU64((u64)scvHtons(hostaddr.sin_port));

  scvBindIPV4(server, (struct sockaddr *)&hostaddr, sizeof(hostaddr), &error);

  if (error.tag) {
    goto error;
  }

  scvListen(server, 128, &error);
  if (error.tag) {
    goto error;
  }

  int clientaddrlen = sizeof(clientaddr);
  
  while (true) {  
    client = scvAccept(server, (struct sockaddr *)&clientaddr, &clientaddrlen, &error);
    if (error.tag) {
      scvPrintError(&error);
      continue;
    }

    int readed = 0;
    readed = scvRead(client, )

    SCVString hw = scvUnsafeCString("HELLO WORLD");
    scvWrite(client, (void *)hw.base, hw.len, &error);
    scvClose(client);

    scvPrintCString("connected");
  }

  return 0;

error:
  scvClose(server);
error1: 
  scvFatalError("fatal error", &error);
  return 1;
}

