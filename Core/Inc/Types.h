/*
 * Types.h
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */
#ifndef TYPES_H
#define TYPES_H

#include "main.h"
#include <stdint.h>
#include <string.h>

// 시스템 상태 정의 (A21~A25 CSU 연계) [cite: 400-403, 456-459]
typedef enum {
    STATE_INIT           = 0x00, // 초기화
    STATE_STANDBY   = 0x01, // 정렬 신호 수신 대기 (STANDBY)
    STATE_ALIGN       = 0x02, // 정렬 중
    STATE_LAUNCH       = 0x03, // 사격 중
    STATE_ERROR          = 0x04  // 고장 상태
} LTL_state_t;

// 제어 및 피드백 데이터 구조체 [cite: 408-411, 460-466]
typedef struct {
    float targetAngle;   // 목표 조준 각도 (0.0~180.0) [cite: 409, 468]
    float currentAngle;  // 현재 모터 각도 [cite: 410, 469]
} LTL_control_t;

// 타이머 및 정렬 상태 관리 [cite: 600, 608-613, 616-618]
typedef struct {
    uint32_t driveStartTimeMs;  // 구동 시작 시간 (HAL_GetTick) [cite: 659]
    uint32_t estimatedArrivalMs; // 도달 예상 소요 시간 [cite: 645]
    uint8_t isAlignComplete;     // 0: 구동중, 1: 완료 [cite: 617, 666]
} LTL_status_t;

// 외부 변수 선언 (각 .c 파일에서 참조)
extern LTL_state_t g_LTL_currentState;
extern LTL_control_t g_LTL_controlData;
extern LTL_status_t g_LTL_status;
extern CAN_HandleTypeDef hcan;
extern TIM_HandleTypeDef htim2;
extern uint8_t rx_data[8];
extern uint8_t uart_rx_buf[8];

#endif
 /* INC_TYPES_H_ */
