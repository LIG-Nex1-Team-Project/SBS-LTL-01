/*
 * can.c
 *
 *  Created on: 2026. 3. 3.
 *      Author: 82109
 */


#include "can.h"
#include "ecs_com.h"

extern CAN_HandleTypeDef hcan;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {

        if(RxHeader.ExtId == 0x00000300) { //

        	receiveDataFromECS();
        }
    }
}

void canInit()
{

	// can 필터 설정.
	CAN_FilterTypeDef sFilterConfig;
	uint32_t filter_id = 0x00000300;      // 내가 받고 싶은 ID
	uint32_t filter_mask = 0x1FFFFFFF;    // 29비트 모든 자리를 다 검사하겠다 (정확히 일치해야 함)

	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;


//	sFilterConfig.FilterIdHigh = (uint16_t)((filter_id << 3) >> 16);
//	sFilterConfig.FilterIdLow  = (uint16_t)((filter_id << 3) | CAN_ID_EXT);
//
//	// 마스크도 ID와 똑같은 위치 규격을 가져야 함
//	sFilterConfig.FilterMaskIdHigh = (uint16_t)((filter_mask << 3) >> 16);
//	sFilterConfig.FilterMaskIdLow  = (uint16_t)((filter_mask << 3) | CAN_ID_EXT);


	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;


	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;



	// can 필터 설정 적용.
	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
		Error_Handler();
	}

	// can 시작
	HAL_CAN_Start(&hcan);

	// 인터럽트 활성화
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

}
