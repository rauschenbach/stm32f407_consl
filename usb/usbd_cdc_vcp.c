#include "usbd_cdc_vcp.h"
#include "main.h"
#include "vcp.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
extern USBD_Class_cb_TypeDef USBD_CDC_cb;



#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif				/* USB_OTG_HS_INTERNAL_DMA_ENABLED */


/*__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END; */




/* These are external variables imported from CDC core to be used for IN transfer management. */
extern uint8_t APP_Rx_Buffer[];	/* Write CDC received data in this buffer.
				   These data will be sent over USB IN endpoint
				   in the CDC core functions. */

extern uint32_t APP_Rx_ptr_in;	/* Increment this pointer or roll it back to
				   start address when writing received data
				   in the buffer APP_Rx_Buffer. */



//static uint8_t  Buf_In[BUF_IN_SIZE];          // буфер приема из СОМ-порта
//static int indxBuf_In_Write = 0, indxBuf_In_Read = 0; // указатели куда писать и откуда читать



static uint16_t VCP_Init(void);
static uint16_t VCP_DeInit(void);
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t * Buf, uint32_t Len);
static uint16_t VCP_DataRx(uint8_t * Buf, uint32_t Len);


/* нужно для вызова из usbd_cdc_core.c*/
CDC_IF_Prop_TypeDef VCP_fops = {
    VCP_Init,
    VCP_DeInit,
    VCP_Ctrl,
    VCP_DataTx,
    VCP_DataRx
};


/**
  * @brief  VCP_Init
  *         Initializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{
    return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{
    return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t * Buf, uint32_t Len)
{
    switch (Cmd) {
    case SEND_ENCAPSULATED_COMMAND:
	/* Not  needed for this driver */
	break;

    case GET_ENCAPSULATED_RESPONSE:
	/* Not  needed for this driver */
	break;

    case SET_COMM_FEATURE:
	/* Not  needed for this driver */
	break;

    case GET_COMM_FEATURE:
	/* Not  needed for this driver */
	break;

    case CLEAR_COMM_FEATURE:
	/* Not  needed for this driver */
	break;

    case SET_LINE_CODING:	//команда настроить контроллер
	//эта команда вызвывается, если на компьютере меняются настройки порта
	break;

    case GET_LINE_CODING:	//команда считать настройки из контроллера
	break;

    case SET_CONTROL_LINE_STATE:
	/* Not  needed for this driver */
	break;

    case SEND_BREAK:
	/* Not  needed for this driver */
	break;

    default:
	break;
    }

    return USBD_OK;
}

/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in 
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
uint16_t VCP_DataTx(uint8_t * Buf, uint32_t Len)
{
    /* буфер APP_Rx_Buffer используется драйвером USB  */
    for (u32 i = 0; i < Len; i++) {
	APP_Rx_Buffer[APP_Rx_ptr_in] = (uint8_t) Buf[i];

	APP_Rx_ptr_in++;	/* increase pointer value */

	/* To avoid buffer overflow */
	if (APP_Rx_ptr_in >= APP_RX_DATA_SIZE) {
	    APP_Rx_ptr_in = 0;
	}
    }
    return USBD_OK;
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  */
uint16_t VCP_DataRx(uint8_t * Buf, uint32_t Len)
{
    vcp_rx_data(Buf, Len);
    return USBD_OK;
}

#if 0
 /* Только COM порт  */
void vcp_init(void)
{
    USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);
}
#endif
