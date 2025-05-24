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
#define PAGE_SIZE 4096
#include "scv.h"
#include "scv_font.h"
#include "scv_geom.h"

u64
scvCntVct(void)
{
  u64 result;
  u32 hi, lo;

  __asm__ __volatile__(
      "rdtsc" : "=a" (lo), "=d" (hi)
  );
  result = (u64)hi << 32 | lo;

  return result;
}

u64
scvGetOSTimerFreq(void)
{
  return 1000000;
}

u64
scvReadOSTimer(void)
{
  struct timeval val;

  scvGetTimeofday(&val, nil);
  u64 result = scvGetOSTimerFreq() * (u64)val.tv_sec + (u64)val.tv_usec;
  return result;
}

u64
scvCntFrq(void) 
{
  static u64 rdtscTicksPerSec = 0; 
  u64 msToWait = 2;
  u64 cpuStart, cpuEnd;
  u64 osStart, osEnd, osElapsed;
  u64 osWaitTime;
  u64 osFreq;

  if (rdtscTicksPerSec == 0) {
    osFreq = scvGetOSTimerFreq();
    osWaitTime = osFreq * msToWait / 1000;
    osStart = scvReadOSTimer();

    osEnd = 0;
    osElapsed = 0;
    cpuStart = scvCntVct(); 

    while (osElapsed < osWaitTime) {
      osEnd = scvReadOSTimer();
      osElapsed = osEnd - osStart;
    }

    cpuEnd = scvCntVct();
    u64 cpuElapsed = cpuEnd - cpuStart;
    u64 cpuFreq = 0;
    if (osElapsed) {
      cpuFreq = osFreq * cpuElapsed / osElapsed;
    }
    rdtscTicksPerSec = cpuFreq;
  }

  return rdtscTicksPerSec;
}

__attribute__ (( __noinline__ )) SCVSyscallResult
scvSyscall(int trap, uptr a1, uptr a2, uptr a3)
{
  SCVSyscallResult r;

  __asm__ __volatile__( 
    "syscall\n"
    "jnc 0f\n"
    "movq $-1, %0\n"
    "movq $0, %1\n"
    "movq %%rax, %2\n"
    "jmp 1f\n"
    "0:\n"
    "movq %%rax, %0\n"
    "movq %%rdx, %1\n"
    "movq $0, %2\n"
    "1:\n"
    :
    "=m" (r.r1), "=m" (r.r2), "=m" (r.err)
    :
    "a"(trap), "D"(a1), "S"(a2), "d"(a3)
    :
    "rcx", "r8", "r9", "r10", "r11"
  );
  return r;
}


__attribute__ (( __noinline__ )) SCVSyscallResult
scvSyscall6(int trap, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5, uptr a6)
{
  SCVSyscallResult r;

  __asm__ __volatile__(
    "movq %7, %%r10\n"
    "movq %8, %%r8\n"
    "movq %9, %%r9\n"
    "syscall\n"
    "jnc 0f\n"
    "movq $-1, %0\n"
    "movq $0, %1\n"
    "movq %%rax, %2\n"
    "jmp 1f\n"
    "0:\n"
    "movq %%rax, %0\n"
    "movq %%rdx, %1\n"
    "movq $0, %2\n"
    "1:\n"
    :
    "=m" (r.r1), "=m" (r.r2), "=m" (r.err)
    :
    "a"(trap), "D"(a1), "S"(a2), "d"(a3), "m" (a4), "m" (a5), "m" (a6)
    :
    "rcx", "r8", "r9", "r10", "r11"
  );

  return r;
}


SCVSlice
_scvLoadFile(SCVString pathname, SCVError *error)
{ 
  struct stat filestat;
  i64 fd = 0;
  void *buf = nil;
  fd = scvOpen(pathname, 0, error);

  SCVSlice s = {0};
  if (fd < 0) {
    scvErrorSet(error, "can't open file", fd); 
    return s;
  } 

  scvFStat(fd, &filestat, error); 

  if (filestat.st_size == 0) {
    return s;
  }

  buf = scvMmap(nil, filestat.st_size, PROT_READ | PROT_WRITE, MAP_FILE, fd, 0, error);
  if (buf == 0) {
    return s;
  }

  return scvUnsafeSlice(buf, filestat.st_size);
}

typedef struct SCVByteSlice SCVByteSlice;
struct SCVByteSlice {
  byte *base;
  u64 len;
  u64 cap;
};

i32
getListenerSocket(SCVError *error)
{
  SCVError error;
  i32 listener;
  i32 yes = 1;
  
  struct sockaddr_in hostaddr;

  listener = scvSocket(PF_INET, SOCK_STREAM, 0, &error);
  if (error->tag) {
    goto error1;
  }

  scvSetSockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  hostaddr.sin_family = AF_INET;
  hostaddr.sin_port   = scvHtons(8888);
  hostaddr.sin_addr.s_addr = INADDR_ANY;

  scvBindIPV4(listener, (struct sockaddr *)&hostaddr, sizeof(hostaddr), &error);
  if (error->tag) {
    goto handle_error;
  }

  scvListen(listener, 128, &error);
  if (error->tag) {
    goto handle_error;
  }

  return listener;

handle_error:
  scvClose(listener);
error1:
  return -1;
}


i32
main(void)
{
#define FD_SIZE 32

  i32 listener;
  i32 newfd;
  i32 fdcount;
  i32 fdsize = FD_SIZE;
  SCVPollFd pfds[FD_SIZE];



}

/*
#define PF_INET 0x2
#define SOCK_STREAM 0x1
#define IPPROTO_IP 0x0
#define AF_INET  0x2
#define INADDR_ANY 0x0
*/
int 
_main(void)
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
  hostaddr.sin_port   = scvHtons(8888);
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

  i32 clientaddrlen = sizeof(clientaddr);

  while (true) {  
    client = scvAccept(server, (struct sockaddr *)&clientaddr, &clientaddrlen, &error);
    if (error.tag) {
      scvPrintError(&error);
      continue;
    }
    
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

