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
//uint8_t rx_data[8];

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
    // 1. 데이터 복사 (Byte 0~3: float 각도)
    float targetAngle;
    memcpy(&targetAngle, &rx_data[0], 4);

    // 2. 명령 모드 추출 (Byte 4: uint8_t 명령)
    uint8_t commandMode = rx_data[4];

    // 3. 명령에 따른 동작 분기
    switch (commandMode) {
        case 0x00: // init (정렬)
            g_LTL_controlData.targetAngle = targetAngle;
            g_LTL_currentState = STATE_ALIGN;
            // 레이저 끄기 (안전을 위해 정렬 중에는 끔)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
            // LED 확인
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            // [모터 구동 핵심 코드]
            // 0~180도 각도를 500~2500 CCR 값으로 변환
            uint32_t ccr_val = 500 + (uint32_t)(targetAngle * (2000.0f / 180.0f));

            // 안전 제한 (0.5ms ~ 2.5ms 범위 준수)
            if (ccr_val < 500)
            	ccr_val = 500;
            if (ccr_val > 2500)
            	ccr_val = 2500;

            // TIM4 CH2(PB7)에 적용
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccr_val);
            break;

        case 0x01: // FIRE (발사)
            // 정렬이 완료된 상태에서만 발사 가능하게 로직 추가 가능
            g_LTL_currentState = STATE_LAUNCH;
            // 레이저 켜기 (PB6 High)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

            break;

        case 0x02: // EMERGENCY (긴급 중지)
            g_LTL_currentState = STATE_ERROR;
            // 모든 구동 정지 및 레이저 끄기
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
            break;

        default:
            break;
    }
}

// A12 CSU: 상태 보고 송신 [cite: 270-272, 439-441, 694]
void sendStateToECS(void) {
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;
    uint8_t tx_data[8] = {0, };

    switch (g_LTL_currentState) {
            case STATE_INIT:
                tx_data[0] = 0x00; // 초기화
                break;
            case STATE_STANDBY:
                tx_data[0] = 0x01; // 정렬 신호 수신 대기
                break;
            case STATE_ALIGN:
                tx_data[0] = 0x02; // 정렬 중
                break;
            case STATE_LAUNCH:
                tx_data[0] = 0x03; // 사격 중
                break;
            case STATE_ERROR:
            default:
                tx_data[0] = 0x04; // 고장 상태
                break;
        }

    // ECS에서 정의한 수신 ID로 변경
    tx_header.ExtId = 0x00000400;
    tx_header.IDE = CAN_ID_EXT;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8;
    tx_header.TransmitGlobalTime = DISABLE;


    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) > 0) {
        HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
    }
}
