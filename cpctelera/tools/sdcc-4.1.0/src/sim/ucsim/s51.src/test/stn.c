char c1;
char *ptr1_data;
char *ptr2_idata;
char *ptr3_pdata;
char *ptr4_xdata;
char *ptr5_code;
char c2;

unsigned char __xdata * volatile sif;

void
main(void)
{
  volatile unsigned int i, j;
  volatile __xdata int xi;
  volatile int __xdata xii;

  sif= (unsigned char __xdata *)0xffff;
  xi= 1;
  xii= 2;
  c1= 'A';
  c2= 'B';
  
  for (j= 0; j<10; j++)
    for (i= 0; i<0xfff0; i++)
      {
	ptr1_data = (__data  char *)(0x1122); // 22 00 40
	ptr2_idata= (__idata char *)(0x3344); // 44 00 40
	ptr3_pdata= (__pdata char *)(0x5566); // 66 00 60
	ptr4_xdata= (__xdata char *)(0x7788); // 88 77 00
	ptr5_code = (__code  char *)(0x99aa); // aa 99 80
      }

  * (char __idata *) 0 = * (char __xdata *) 0xfffe;
  *sif= 's';
}
