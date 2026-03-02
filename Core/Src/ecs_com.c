/*
 * ecs_com.c
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */

#include "types.h"
#include "ecs_com.h"  // 자신의 헤더 포함
#include <string.h>

CAN_RxHeaderTypeDef rx_header;
uint8_t rx_data[8];

// A11 CSU: 데이터 수신 및 변환 [cite: 265-266, 427-428]
void receiveDataFromECS(void) {
    if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
            if (rx_header.ExtId == 0x00000300) { // ECS 송신 ID 필터링 [cite: 698]
                convertDataToLTLStruct();
            }
        }
    }
}

void convertDataToLTLStruct(void) {
    // Byte 0~3: float Target_Degree (Little-Endian) [cite: 687, 701]
    memcpy(&g_LTL_controlData.targetAngle, &rx_data[0], sizeof(float));

    // Byte 4: Fire_Command [cite: 687]
    uint8_t cmd = rx_data[4];
    if (cmd == 0x00) g_LTL_currentState = STATE_ALIGN;
    else if (cmd == 0x01) g_LTL_currentState = STATE_LAUNCH;
    else if (cmd == 0x02) g_LTL_currentState = STATE_ERROR;
}

// A12 CSU: 상태 보고 송신 [cite: 270-272, 439-441, 694]
void sendStateToECS(void) {
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;
    uint8_t tx_status[8] = {0,};

    // 상태 매핑: 0x00:초기화, 0x01:정렬중, 0x02:정렬완료, 0x03:사격중, 0x04:고장 [cite: 687]
    if (g_LTL_currentState == STATE_INIT) tx_status[0] = 0x00;
    else if (g_LTL_currentState == STATE_ALIGN) tx_status[0] = 0x01;
    else if (g_LTL_currentState == STATE_LAUNCH) tx_status[0] = 0x03;
    else if (g_LTL_currentState == STATE_ERROR) tx_status[0] = 0x04;
    else tx_status[0] = (g_LTL_status.isAlignComplete) ? 0x02 : 0x00;

    tx_header.ExtId = 0x18888408; // 발사대 송신 ID [cite: 699]
    tx_header.IDE = CAN_ID_EXT;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8;

    HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_status, &tx_mailbox);
}
