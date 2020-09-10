//#include <stdio.h>

unsigned long read_cycles(void)
{
  unsigned long cycles;
  asm volatile ("LABEL: rdcycle %0" : "=r" (cycles));
  return cycles;
}

void li(void)
{
  asm volatile("li x10 0xabcdefaa");
}

int main()
{
    //printf("Hello world\n");
    int a=3;
    int b=3;
    printf("Hello world\n");
   
    int sum = 0;
    unsigned long start = read_cycles();
    for (int i=0;i<4096; i++) sum+=i;
    unsigned long end = read_cycles();
    li();
    printf("Sum is %d\n", sum);
    printf("Loop took %d cycles\n", end-start);
    return a+b;
}
