typedef struct SCVByteSlice SCVByteSlice;
struct SCVByteSlice {
  char *base;
  unsigned long len;
  unsigned long cap;
};

SCVByteSlice
scvEmptySlice()
{
  SCVByteSlice s = {0};
  return s;
}

int
main(int argc, char **argv) 
{
  (void)argc;
  (void)argv;
  SCVByteSlice sl = scvEmptySlice();
  return sl.len + sl.cap;
}

void
Exit(unsigned long status)
{
  __asm__ __volatile__(
      "movq $60, %%rax\n"
      "syscall\n"
      :
      : "D"(status)
      :
  );
}

void
_start()
{
  SCVByteSlice sl = scvEmptySlice();
  Exit(sl.len + sl.cap);
}
