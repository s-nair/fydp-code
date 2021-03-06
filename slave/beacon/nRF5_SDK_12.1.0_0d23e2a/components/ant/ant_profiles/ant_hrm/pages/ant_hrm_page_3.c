/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "sdk_config.h"
#if ANT_HRM_ENABLED

#include "ant_hrm_page_3.h"

#define NRF_LOG_MODULE_NAME "ANT_HRM_PAGE_3"
#if ANT_HRM_PAGE_3_LOG_ENABLED
#define NRF_LOG_LEVEL       ANT_HRM_PAGE_3_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  ANT_HRM_PAGE_3_INFO_COLOR
#else // ANT_HRM_PAGE_3_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // ANT_HRM_PAGE_3_LOG_ENABLED
#include "nrf_log.h"

/**@brief HRM page 3 data layout structure. */
typedef struct
{
    uint8_t hw_version;
    uint8_t sw_version;
    uint8_t model_num;
    uint8_t reserved[4];
}ant_hrm_page3_data_layout_t;

/**@brief Function for tracing page 3 and common data.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[in]  p_page_data      Pointer to the page 3 data.
 */
static void page3_data_log(ant_hrm_page3_data_t const * p_page_data)
{
    NRF_LOG_INFO("Hardware Rev ID                   %u\r\n", (unsigned int)p_page_data->hw_version);
    NRF_LOG_INFO("Model                             %u\r\n", (unsigned int)p_page_data->model_num);
    NRF_LOG_INFO("Software Ver ID                   %u\r\n\n", (unsigned int)p_page_data->sw_version);
}


void ant_hrm_page_3_encode(uint8_t                    * p_page_buffer,
                           ant_hrm_page3_data_t const * p_page_data)
{
    ant_hrm_page3_data_layout_t * p_outcoming_data = (ant_hrm_page3_data_layout_t *)p_page_buffer;

    p_outcoming_data->hw_version = (uint8_t)p_page_data->hw_version;
    p_outcoming_data->sw_version = (uint8_t)p_page_data->sw_version;
    p_outcoming_data->model_num  = (uint8_t)p_page_data->model_num;

    page3_data_log(p_page_data);
}


void ant_hrm_page_3_decode(uint8_t const        * p_page_buffer,
                           ant_hrm_page3_data_t * p_page_data)
{
    ant_hrm_page3_data_layout_t const * p_incoming_data =
        (ant_hrm_page3_data_layout_t *)p_page_buffer;

    p_page_data->hw_version = (uint32_t)p_incoming_data->hw_version;
    p_page_data->sw_version = (uint32_t)p_incoming_data->sw_version;
    p_page_data->model_num  = (uint32_t)p_incoming_data->model_num;

    page3_data_log(p_page_data);
}

#endif // ANT_HRM_ENABLED
