#include <stdio.h>

unsigned long read_cycles(void)
{
  unsigned long cycles;
  asm volatile ("rdcycle %0" : "=r" (cycles));
  return cycles;
}

float f = 3.1415;

int main()
{
    unsigned long s, e;
    s = read_cycles();
    float g = 2*f;
    for (int i=0;i<1024;i++) {
         g+=1.0;
    }
    e = read_cycles();
    printf("%f in %ld \n", g, e-s);
    return 0;
}
