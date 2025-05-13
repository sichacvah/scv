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

