#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include <zmk/pointing/iqs9151_split_scroll_inertia.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) &&                                                  \
    IS_ENABLED(CONFIG_INPUT_IQS9151_SCROLL_INERTIA_CANCEL_ON_MODIFIERS)

static atomic_t iqs9151_split_scroll_cancel_generation = ATOMIC_INIT(0);
static atomic_t iqs9151_split_scroll_cancel_started_ms = ATOMIC_INIT(0);

void zmk_iqs9151_split_scroll_inertia_request_cancel(void) {
    atomic_set(&iqs9151_split_scroll_cancel_started_ms, (atomic_val_t)k_uptime_get_32());
    atomic_inc(&iqs9151_split_scroll_cancel_generation);
}

uint32_t zmk_iqs9151_split_scroll_inertia_cancel_generation(void) {
    return (uint32_t)atomic_get(&iqs9151_split_scroll_cancel_generation);
}

uint32_t zmk_iqs9151_split_scroll_inertia_cancel_started_ms(void) {
    return (uint32_t)atomic_get(&iqs9151_split_scroll_cancel_started_ms);
}

#else

void zmk_iqs9151_split_scroll_inertia_request_cancel(void) {}

uint32_t zmk_iqs9151_split_scroll_inertia_cancel_generation(void) { return 0U; }

uint32_t zmk_iqs9151_split_scroll_inertia_cancel_started_ms(void) { return 0U; }

#endif
