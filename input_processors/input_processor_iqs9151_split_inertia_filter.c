/*
 * Input processor that strips IQS9151 split inertia tags and suppresses
 * tagged scroll events immediately after a modifier-triggered cancel request.
 */

#define DT_DRV_COMPAT zmk_input_processor_iqs9151_split_inertia_filter

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/kernel.h>

#include <drivers/input_processor.h>

#include <zmk/pointing/iqs9151_split_scroll_inertia.h>

struct iqs9151_split_inertia_filter_data {
    uint32_t cancel_generation;
    uint32_t cancel_started_ms;
    bool cancel_active;
};

static bool iqs9151_is_scroll_event(const struct input_event *event) {
    return event->type == INPUT_EV_REL &&
           (event->code == INPUT_REL_WHEEL || event->code == INPUT_REL_HWHEEL);
}

static void iqs9151_neutralize_event(struct input_event *event) {
    event->type = INPUT_EV_KEY;
    event->code = INPUT_BTN_8;
    event->value = 0;
}

static void iqs9151_sync_cancel_request(struct iqs9151_split_inertia_filter_data *data,
                                        uint32_t now_ms) {
    const uint32_t cancel_generation = zmk_iqs9151_split_scroll_inertia_cancel_generation();

    if (cancel_generation == data->cancel_generation) {
        return;
    }

    data->cancel_generation = cancel_generation;
    data->cancel_started_ms = zmk_iqs9151_split_scroll_inertia_cancel_started_ms();
    data->cancel_active = cancel_generation != 0U &&
                          (uint32_t)(now_ms - data->cancel_started_ms) <
                              IQS9151_SPLIT_SCROLL_CANCEL_HOLD_MS;
}

static bool iqs9151_cancel_is_active(struct iqs9151_split_inertia_filter_data *data,
                                     uint32_t now_ms) {
    if (!data->cancel_active) {
        return false;
    }

    if ((uint32_t)(now_ms - data->cancel_started_ms) >= IQS9151_SPLIT_SCROLL_CANCEL_HOLD_MS) {
        data->cancel_active = false;
        return false;
    }

    return true;
}

static void iqs9151_note_manual_scroll(struct iqs9151_split_inertia_filter_data *data,
                                       uint32_t now_ms) {
    if (!iqs9151_cancel_is_active(data, now_ms)) {
        return;
    }

    if ((uint32_t)(now_ms - data->cancel_started_ms) < IQS9151_SPLIT_SCROLL_MANUAL_CLEAR_GUARD_MS) {
        return;
    }

    data->cancel_active = false;
}

static int iqs9151_split_inertia_filter_handle_event(const struct device *dev,
                                                     struct input_event *event, uint32_t param1,
                                                     uint32_t param2,
                                                     struct zmk_input_processor_state *state) {
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    struct iqs9151_split_inertia_filter_data *data = dev->data;
    const uint32_t now_ms = k_uptime_get_32();

    iqs9151_sync_cancel_request(data, now_ms);

    if (!iqs9151_is_scroll_event(event)) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (!zmk_iqs9151_split_scroll_inertia_is_encoded(event->value)) {
        iqs9151_note_manual_scroll(data, now_ms);
        return ZMK_INPUT_PROC_CONTINUE;
    }

    event->value = zmk_iqs9151_split_scroll_inertia_decode(event->value);

    if (!iqs9151_cancel_is_active(data, now_ms)) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    iqs9151_neutralize_event(event);

    return ZMK_INPUT_PROC_CONTINUE;
}

static struct zmk_input_processor_driver_api iqs9151_split_inertia_filter_driver_api = {
    .handle_event = iqs9151_split_inertia_filter_handle_event,
};

static int iqs9151_split_inertia_filter_init(const struct device *dev) {
    ARG_UNUSED(dev);
    return 0;
}

#define IQS9151_SPLIT_INERTIA_FILTER_INST(n)                                                       \
    static struct iqs9151_split_inertia_filter_data iqs9151_split_inertia_filter_data_##n;         \
    DEVICE_DT_INST_DEFINE(n, &iqs9151_split_inertia_filter_init, NULL,                            \
                          &iqs9151_split_inertia_filter_data_##n, NULL, POST_KERNEL,              \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                     \
                          &iqs9151_split_inertia_filter_driver_api);

DT_INST_FOREACH_STATUS_OKAY(IQS9151_SPLIT_INERTIA_FILTER_INST)
