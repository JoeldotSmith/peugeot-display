#include "ui.h"

lv_obj_t *ui____initial_actions0 = NULL;
void ui_init(void) {
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(
        dispp,
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        true,
        LV_FONT_DEFAULT
    );
    lv_display_set_theme(dispp, theme);

    ui_Screen1_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_screen_load(ui_Screen1);
}

void ui_destroy(void) {
    ui_Screen1_screen_destroy();
}
