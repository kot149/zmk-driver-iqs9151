#pragma once

#ifndef ZMK_POINTING_IQS9151_SPLIT_SCROLL_INERTIA_H_
#define ZMK_POINTING_IQS9151_SPLIT_SCROLL_INERTIA_H_

#include <stdbool.h>
#include <stdint.h>

#define IQS9151_SPLIT_SCROLL_INERTIA_TAG 0x49510000UL
#define IQS9151_SPLIT_SCROLL_INERTIA_VALUE_MASK 0x0000FFFFUL
#define IQS9151_SPLIT_SCROLL_CANCEL_HOLD_MS 3500U
#define IQS9151_SPLIT_SCROLL_MANUAL_CLEAR_GUARD_MS 200U

static inline bool zmk_iqs9151_split_scroll_inertia_is_encoded(int32_t value) {
    return (((uint32_t)value) & 0xFFFF0000UL) == IQS9151_SPLIT_SCROLL_INERTIA_TAG;
}

static inline int32_t zmk_iqs9151_split_scroll_inertia_encode(int16_t value) {
    return (int32_t)(IQS9151_SPLIT_SCROLL_INERTIA_TAG | (uint16_t)value);
}

static inline int16_t zmk_iqs9151_split_scroll_inertia_decode(int32_t value) {
    return (int16_t)(((uint32_t)value) & IQS9151_SPLIT_SCROLL_INERTIA_VALUE_MASK);
}

void zmk_iqs9151_split_scroll_inertia_request_cancel(void);
uint32_t zmk_iqs9151_split_scroll_inertia_cancel_generation(void);
uint32_t zmk_iqs9151_split_scroll_inertia_cancel_started_ms(void);

#endif /* ZMK_POINTING_IQS9151_SPLIT_SCROLL_INERTIA_H_ */
