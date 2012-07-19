#ifndef ___IRQ_H___
#define ___IRQ_H___

#define MAX_IRQ 256

int irq_cnt_begin(int irq);
int irq_get_cnt(int id);
void irq_cnt_end(int id);

#endif  /* ___IRQ_H___ */


