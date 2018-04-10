# ifndef _SETJMP_H
# define _SETJMP_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct __jmp_buf {
  unsigned long __eax, __ebx, __ecx, __edx, __esi;
  unsigned long __edi, __ebp, __esp, __eip, __eflags;
  unsigned short __cs, __ds, __es, __fs, __gs, __ss;
  unsigned long __sigmask; /* for POSIX signals only */
  unsigned long __signum; /* for expansion */
  unsigned long __exception_ptr; /* pointer to previous exception */
  unsigned char __fpu_state[108]; /* for future use */
} jmp_buf[1];

extern "C" void longjmp(jmp_buf env, int val);
extern "C" int	setjmp(jmp_buf env);

/*
jmp_buf a, b;
  int i;

  printf("test of setjmp/longjmp\n");

  i = setjmp(a);

  printf("\n");
  printf("ax=%08lx bx=%08lx cx=%08lx dx=%08lx si=%08lx di=%08lx\n",
         a->__eax, a->__ebx, a->__ecx, a->__edx, a->__esi, a->__edi);
  printf("cs:eip=%04x:%08lx bp=%08lx ss:esp=%04x:%08lx\n",
         a->__cs, a->__eip, a->__ebp, a->__ss, a->__esp);
  printf("cs=%04x ds=%04x es=%04x fs=%04x gs=%04x ss=%04x fl=%08lx\n",
         a->__cs, a->__ds, a->__es, a->__fs, a->__gs, a->__ss, a->__eflags);

  //if (i == 0)
    longjmp(a, 1);
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
