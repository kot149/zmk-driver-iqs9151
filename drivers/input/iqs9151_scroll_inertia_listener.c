#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>
#include <zmk/pointing/iqs9151.h>
#include <zmk/pointing/iqs9151_split_scroll_inertia.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_INPUT_IQS9151_SCROLL_INERTIA_CANCEL_ON_MODIFIERS)

static zmk_mod_flags_t iqs9151_modifier_flag(uint32_t keycode) {
    return (zmk_mod_flags_t)BIT(keycode - HID_USAGE_KEY_KEYBOARD_LEFTCONTROL);
}

static zmk_mod_flags_t iqs9151_scroll_inertia_cancel_modifiers(void) {
    zmk_mod_flags_t modifiers = 0;

#if IS_ENABLED(CONFIG_INPUT_IQS9151_SCROLL_INERTIA_CANCEL_ON_CTRL)
    modifiers |= iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_LEFTCONTROL) |
                 iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_RIGHTCONTROL);
#endif
#if IS_ENABLED(CONFIG_INPUT_IQS9151_SCROLL_INERTIA_CANCEL_ON_SHIFT)
    modifiers |= iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_LEFTSHIFT) |
                 iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_RIGHTSHIFT);
#endif
#if IS_ENABLED(CONFIG_INPUT_IQS9151_SCROLL_INERTIA_CANCEL_ON_ALT)
    modifiers |= iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_LEFTALT) |
                 iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_RIGHTALT);
#endif
#if IS_ENABLED(CONFIG_INPUT_IQS9151_SCROLL_INERTIA_CANCEL_ON_GUI)
    modifiers |= iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_LEFT_GUI) |
                 iqs9151_modifier_flag(HID_USAGE_KEY_KEYBOARD_RIGHT_GUI);
#endif

    return modifiers;
}

static bool iqs9151_is_configured_cancel_keycode(const struct zmk_keycode_state_changed *ev) {
    if (!is_mod(ev->usage_page, ev->keycode)) {
        return false;
    }

    return (iqs9151_scroll_inertia_cancel_modifiers() & iqs9151_modifier_flag(ev->keycode)) != 0U;
}

static bool iqs9151_should_cancel_scroll_inertia(const struct zmk_keycode_state_changed *ev) {
    if (ev == NULL || !ev->state) {
        return false;
    }

    const zmk_mod_flags_t configured_modifiers = iqs9151_scroll_inertia_cancel_modifiers();
    const zmk_mod_flags_t active_modifiers = ev->implicit_modifiers | ev->explicit_modifiers;

    if ((active_modifiers & configured_modifiers) != 0) {
        return true;
    }

    return iqs9151_is_configured_cancel_keycode(ev);
}

static int iqs9151_handle_scroll_inertia_keycode(const struct zmk_keycode_state_changed *ev) {
    if (!iqs9151_should_cancel_scroll_inertia(ev)) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    LOG_DBG("cancel scroll inertia for keycode page=0x%x code=0x%x", ev->usage_page,
            (unsigned int)ev->keycode);
    zmk_iqs9151_split_scroll_inertia_request_cancel();
    iqs9151_cancel_all_scroll_inertia();

    return ZMK_EV_EVENT_BUBBLE;
}

#ifdef CONFIG_INPUT_IQS9151_TEST
int iqs9151_test_handle_scroll_inertia_keycode(const struct zmk_keycode_state_changed *ev) {
    return iqs9151_handle_scroll_inertia_keycode(ev);
}
#endif

static int iqs9151_scroll_inertia_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);

    return iqs9151_handle_scroll_inertia_keycode(ev);
}

ZMK_LISTENER(iqs9151_scroll_inertia, iqs9151_scroll_inertia_listener);
ZMK_SUBSCRIPTION(iqs9151_scroll_inertia, zmk_keycode_state_changed);

#endif
