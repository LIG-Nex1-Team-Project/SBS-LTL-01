/*
 * laying.c
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */

#include "types.h"
#include "laying.h"   // 자신의 헤더 포함
#include "ecs_com.h"  // sendStateToECS() 사용을 위해 필요
#include <math.h>

/* ... 기존 코드 ... */

LTL_control_t g_LTL_controlData;
LTL_status_t g_LTL_status;

// A31 CSU: 구동 시간 계산 (60도당 100ms + 여유율 1.5) [cite: 303-304, 623, 630-645]
uint32_t calculateDriveTime(void) {
    float delta = fabsf(g_LTL_controlData.targetAngle - g_LTL_controlData.currentAngle);
    g_LTL_status.estimatedArrivalMs = (uint32_t)(delta * (100.0f / 60.0f) * 1.5f);
    return g_LTL_status.estimatedArrivalMs;
}

// A32 CSU: SG90 PWM 출력 (PB7/D9, 64MHz 클럭 기준) [cite: 307-309, 647, 656]
void updateServoPWM(void) {
    // 0~180도 -> 1000~2000 Pulse (1ms~2ms) [cite: 541, 650]
    uint32_t pulse = 700 + (uint32_t)(g_LTL_controlData.targetAngle * (1900.0f / 180.0f));
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse);
    g_LTL_status.driveStartTimeMs = HAL_GetTick();
}

// A33 CSU: 정렬 완료 판정 [cite: 311, 314-315, 661, 669]
void checkAlignmentStatus(void) {
    if (HAL_GetTick() - g_LTL_status.driveStartTimeMs >= g_LTL_status.estimatedArrivalMs) {
        g_LTL_status.isAlignComplete = 1;
        g_LTL_controlData.currentAngle = g_LTL_controlData.targetAngle;
    }
}

// A34 CSU: KY-008 레이저 제어 (PB6/D10, 1초 제한) [cite: 317-319, 671, 680]
void controlLaserLaunch(void) {
    static uint32_t startTick = 0;
    static uint8_t active = 0;

    if (!active) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // 점등 [cite: 570]
        startTick = HAL_GetTick();
        active = 1;
        sendStateToECS();
    }

    if (HAL_GetTick() - startTick >= 1000) { // 1000ms 유지 [cite: 289, 319, 680]
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // 소등
        active = 0;
        g_LTL_currentState = STATE_STANDBY;
        g_LTL_status.isAlignComplete = 0;
        sendStateToECS();
    }
}
