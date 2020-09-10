//#include <stdio.h>

unsigned long read_cycles(void)
{
  unsigned long cycles;
  asm volatile ("rdcycle %0" : "=r" (cycles));
  return cycles;
}

#if 0
void vsetvli(void)
{
  asm volatile ("vsetvli a3, a0, e32, m8"); 
}
#endif

void vadd(void)
{
  asm volatile ("vadd.vv v1, v2, v3, vm");
}

void add(void)
{
  asm volatile("add a2, a2, t1");
}

int main()
{
    //printf("Hello world\n");
    int a=3;
    int b=3;
    printf("Hello world\n");

    //vsetvli();
    add();

    printf("Hello again\n");
   
    return a+b;
}
