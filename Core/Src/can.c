#include "can.h"
#include "main.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

bool can1_enter_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    (void)RCC->APB1ENR;
    CAN1->MCR |= CAN_MCR_INRQ;
    CAN1->MCR &= ~CAN_MCR_SLEEP;
    uint32_t start = HAL_GetTick();
    while ((CAN1->MSR & CAN_MSR_INAK) == 0u)
    {
        if ((HAL_GetTick() - start) >= 100u)
        {
            return false;
        }
    }

    return true;
}

void can1_configure_loopback(void){
    CAN1->BTR = 3u | (15u << 16) | (3u <<20) | (1u << 30) | (1u << 31);
}
void can1_configure_500k_normal(void)
{
    CAN1->BTR =
        3u |
        (15u << 16u) |
        (3u << 20u);
}

void can1_configure_filter(void)
{
    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FA1R &= ~(1u);
    CAN1->FM1R &= ~(1u);
    CAN1->FS1R |= (1u);
    CAN1->sFilterRegister[0].FR1 = 0u;
    CAN1->sFilterRegister[0].FR2 = 0u;
    CAN1->FFA1R &= ~(1u);
    CAN1->FA1R |= (1u);
    CAN1->FMR &= ~CAN_FMR_FINIT;

}

bool can1_start(void)
{
    CAN1->MCR &= ~CAN_MCR_INRQ;

    uint32_t start = HAL_GetTick();
    while(CAN1->MSR & CAN_MSR_INAK){
        if((HAL_GetTick() - start ) >= 100u){
            return false;
        }
    }
    return true;
}

bool can1_try_send(const CanFrame *frame)
{

    if(frame == NULL || frame->length > 8 || frame->id > 0x7FF)
    {
        return false;
    }
    if(CAN1->TSR & CAN_TSR_TME0){
        uint32_t low = 0u;
        uint32_t high = 0u;
        for(int i = 0; i < frame->length; i++){
            if(i < 4){
                low |= ((uint32_t) frame->data[i] << 8*i);
            } else{
                high |= ((uint32_t) frame->data[i] << (8*i)%32);
            }
        }
        CAN1->sTxMailBox[0].TDLR = low;
        CAN1->sTxMailBox[0].TDHR = high;

        CAN1->sTxMailBox[0].TIR = (frame->id << 21);
        CAN1->sTxMailBox[0].TDTR = frame->length;
         CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;
        return true;
    }
    return false;
}
// need to loop through fifo and store data in *out
bool can1_try_receive(CanFrame *out)
{
    
    if(out == NULL || (CAN1->RF0R & CAN_RF0R_FMP0) == 0u)
    {
        return false;
    }
    uint32_t length = CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC;
    uint32_t identifier_register = CAN1->sFIFOMailBox[0].RIR;

    if (length > 8u || (identifier_register & (CAN_RI0R_IDE | CAN_RI0R_RTR)) != 0u)
    {
        CAN1->RF0R = CAN_RF0R_RFOM0;
        return false;
    }

    for(int i = 0; i < length; i++){
        if(i < 4)
        {
           out->data[i] = (uint8_t)((CAN1->sFIFOMailBox[0].RDLR >> (8u * i)) & 0xFFu);
        } 
        else{
            out->data[i] = (uint8_t)((CAN1->sFIFOMailBox[0].RDHR >> ((8u * i)%32u)) & 0xFFu);
        }
    }

    out->id =(uint16_t)((identifier_register >> 21) & 0x7FFu);
    out->length = (uint8_t) length;
    CAN1->RF0R = CAN_RF0R_RFOM0;
    return true;
}