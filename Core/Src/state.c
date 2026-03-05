/*
 * state.c
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */

#include "Types.h"
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
/*
 *STATE_INIT           = 0x00, // 초기화
    STATE_READY_ALIGN    = 0x01, // 정렬 신호 수신 대기 (STANDBY)
    STATE_ALIGN       = 0x02, // 정렬 중
    STATE_FIRING         = 0x03, // 사격 중
    STATE_ERROR          = 0x04  // 고장 상태
 */

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

// 1. 정렬 프로세스: 시작될 때 레이저를 무조건 끕니다.
void executeAlignProcess(void) {
    static uint8_t started = 0;
    if (!started) {
        // ⭐ [추가] 정렬(모터 이동) 시작 시 레이저를 끕니다.
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        printf("[ALIGN] Motor Moving - Laser OFF\n");

        calculateDriveTime();
        updateServoPWM();
        started = 1;
    }

    checkAlignmentStatus();

    if (g_LTL_status.isAlignComplete) {
        // UI 사격 버튼 활성화를 위해 상태 유지
        started = 0;
    }
}

void executeLaunchProcess(void) {
    static uint32_t launchStartTime = 0;
    static uint8_t isFiring = 0;

    // 1. 사격 명령이 처음 들어온 시점
    if (!isFiring) {
        if (g_LTL_status.isAlignComplete) {
            // 레이저 ON (PB6)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
            launchStartTime = HAL_GetTick(); // 현재 시간 저장
            isFiring = 1;
            printf("[FIRE] Laser ON - Firing for 2 second...\n");
        } else {
            g_LTL_currentState = STATE_ERROR;
            return;
        }
    }

    // 2. 1초(1000ms)가 경과했는지 체크
    if (HAL_GetTick() - launchStartTime >= 2000) {
        isFiring = 0; // 플래그 초기화
        printf("[FIRE] 1 second passed. Returning to ALIGN state to unlock UI.\n");

        // 💡 핵심: 상태를 ALIGN으로 변경하여 UI 비활성화를 해제함
        g_LTL_currentState = STATE_ALIGN;

        // 레이저는 꺼지지 않고 유지됩니다.
        // (이후 새로운 Align 명령이 들어와 모터가 움직일 때 꺼짐)
    }
}

void executeErrorProcess(void) {
    // 긴급 차단 및 Failsafe [cite: 291-292, 571-572, 583]
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // 레이저 OFF
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1500);   // 모터 중립 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccr_val);
    sendStateToECS();
}
