#ifndef PRINT_HEADER
#define PRINT_HEADER


extern void print(char *s) __reentrant;
extern void print_form(char *s, long l, void *p) __reentrant;
#define print_f(A,B) print_form((A),(B),0)
#define print_fp(A,B) print_form((A),0,(B))
extern void print_d(long i) __reentrant;
extern void print_u(unsigned int i) __reentrant;
extern void print_cx(unsigned char i);
extern void print_x(unsigned int i) __reentrant;
extern void print_lx(unsigned long i) __reentrant;
extern void print_p(void *p) __reentrant;
extern void print_c(char c);

extern void term_cls();
extern void term_xy(char x1, char y1);
extern void term_save();
extern void term_restore();
extern void term_hide();
extern void term_show();
extern void term_color(int bg, int fg) __reentrant;

  
#endif
