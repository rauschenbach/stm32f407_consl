#ifndef __USBD_CDC_VCP_H
#define __USBD_CDC_VCP_H


#include "stm32f4xx.h"
#include "usbd_cdc_core.h"
#include "usbd_conf.h"


/* Exported typef ------------------------------------------------------------*/
/* The following structures groups all needed parameters to be configured for the 
   ComPort. These parameters can modified on the fly by the host through CDC class
   command class requests. */
typedef struct {
    uint32_t bitrate;
    uint8_t format;
    uint8_t paritytype;
    uint8_t datatype;
} LINE_CODING;

/* Exported constants --------------------------------------------------------*/

#define DEFAULT_CONFIG                  0
#define OTHER_CONFIG                    1


uint16_t VCP_DataTx(uint8_t * Buf, uint32_t Len);
void vcp_init(void);

#endif				/* __USBD_CDC_VCP_H */
