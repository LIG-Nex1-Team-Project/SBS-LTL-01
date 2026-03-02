/*
 * state.c
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */

#include "types.h"
#include "state.h"    // 자신의 헤더를 포함하여 함수 프로토타입 선언 확인
#include "ecs_com.h"  // sendStateToECS() 사용을 위해 필요
#include "laying.h"   // calculateDriveTime() 등 사용을 위해 필요

LTL_state_t g_LTL_currentState = STATE_INIT;

// A20 CSU: 상태 머신 실행 [cite: 498-499, 506]
void executeStateProcess(void) {
    switch (g_LTL_currentState) {
        case STATE_INIT:    executeInitProcess(); break;
        case STATE_STANDBY: executeStanbyProcess(); break;
        case STATE_ALIGN:   executeAlignProcess(); break;
        case STATE_LAUNCH:  executeLaunchProcess(); break;
        case STATE_ERROR:   executeErrorProcess(); break;
    }
}

void executeInitProcess(void) {
    // 변수 및 HW 초기화 로직 [cite: 277-278, 520]
    g_LTL_status.isAlignComplete = 0;
    g_LTL_controlData.currentAngle = 0.0f;
    g_LTL_currentState = STATE_STANDBY;
}

void executeStanbyProcess(void) {
    // 명령 수신 대기 및 주기적 보고 [cite: 279-281, 534, 536]
    static uint32_t lastTick = 0;
    if (HAL_GetTick() - lastTick >= 50) {
        sendStateToECS();
        lastTick = HAL_GetTick();
    }
}

void executeAlignProcess(void) {
    // 정렬 수행 및 감시 [cite: 283-284, 538, 547]
    static uint8_t started = 0;
    if (!started) {
        calculateDriveTime();
        updateServoPWM();
        started = 1;
    }
    checkAlignmentStatus();
    if (g_LTL_status.isAlignComplete) {
        started = 0;
        g_LTL_currentState = STATE_STANDBY;
        sendStateToECS();
    }
}

void executeLaunchProcess(void) {
    // 사격 통제 제어 [cite: 287-289, 562, 570]
    if (g_LTL_status.isAlignComplete) {
        controlLaserLaunch();
    } else {
        g_LTL_currentState = STATE_ERROR;
    }
}

void executeErrorProcess(void) {
    // 긴급 차단 및 Failsafe [cite: 291-292, 571-572, 583]
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // 레이저 OFF
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1500);   // 모터 중립
    sendStateToECS();
}
