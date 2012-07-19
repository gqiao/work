#ifndef __DMA_H__
#define __DMA_H__

int dma_init(struct device *dev);
int dma_test_start(unsigned pages);
int dma_exit(void);

#endif	/* __DMA_H__ */


