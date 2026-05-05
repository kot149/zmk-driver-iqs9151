#pragma once

#ifndef ZMK_POINTING_IQS9151_H_
#define ZMK_POINTING_IQS9151_H_

#include <zephyr/device.h>

void iqs9151_cancel_scroll_inertia(const struct device *dev);
void iqs9151_cancel_all_scroll_inertia(void);

#endif /* ZMK_POINTING_IQS9151_H_ */
