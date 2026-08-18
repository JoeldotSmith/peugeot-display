#include "ui.h"

lv_obj_t *ui_Screen1 = NULL;
lv_obj_t *ui_Panel1 = NULL;
lv_obj_t *ui_Container1 = NULL;
lv_obj_t *ui_Container4 = NULL;
lv_obj_t *ui_Label1 = NULL;
lv_obj_t *ui_Label2 = NULL;
lv_obj_t *ui_Container2 = NULL;
lv_obj_t *ui_Chart1 = NULL;
lv_obj_t *ui_Container3 = NULL;
lv_obj_t *ui_Container5 = NULL;
lv_obj_t *boostLabel = NULL;
lv_obj_t *oilPressureLabel = NULL;
lv_obj_t *manifoldPressureLabel = NULL;
lv_obj_t *boostBar = NULL;
lv_obj_t *boostUnitLabel = NULL;
lv_obj_t *rpmLabel = NULL;
lv_obj_t *indicatorLabel = NULL;
lv_obj_t *spoilerLabel = NULL;
lv_obj_t *canStatusLabel = NULL;

// colour palette
#define COL_BG          0x0A0A0A
#define COL_PANEL_BG    0x111111
#define COL_BORDER      0x2A2A2A
#define COL_ACCENT      0xFAAE00
#define COL_DIM         0x7A5500
#define COL_DIVIDER     0x222222
#define COL_LEFT_BG     0x0D0D0D
#define COL_RIGHT_BG    0x0F0F0F
#define COL_BAR_BG      0x1A0A00
#define COL_BAR_FG      0xFAAE00

void ui_Screen1_screen_init(void)
{
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(COL_BG), 0);

    // ── Main panel ───────────────────────────────────────────────
    ui_Panel1 = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_Panel1, 1024, 328);
    lv_obj_set_pos(ui_Panel1, 0, 6);
    lv_obj_set_align(ui_Panel1, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_Panel1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_Panel1, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_Panel1, lv_color_hex(COL_PANEL_BG), 0);
    lv_obj_set_style_bg_opa(ui_Panel1, 255, 0);
    lv_obj_set_style_border_color(ui_Panel1, lv_color_hex(COL_BORDER), 0);
    lv_obj_set_style_border_width(ui_Panel1, 2, 0);
    lv_obj_set_style_border_opa(ui_Panel1, 255, 0);
    lv_obj_set_style_radius(ui_Panel1, 8, 0);
    lv_obj_set_style_pad_all(ui_Panel1, 14, 0);
    lv_obj_set_style_pad_row(ui_Panel1, 10, 0);
    lv_obj_set_style_shadow_color(ui_Panel1, lv_color_hex(COL_BORDER), 0);
    lv_obj_set_style_shadow_width(ui_Panel1, 12, 0);
    lv_obj_set_style_shadow_opa(ui_Panel1, 60, 0);

    // ── Top row ──────────────────────────────────────────────────
    ui_Container2 = lv_obj_create(ui_Panel1);
    lv_obj_remove_style_all(ui_Container2);
    lv_obj_set_size(ui_Container2, 971, 230);
    lv_obj_set_align(ui_Container2, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_Container2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_Container2, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    // ── Left panel ───────────────────────────────────────────────
    ui_Container1 = lv_obj_create(ui_Container2);
    lv_obj_remove_style_all(ui_Container1);
    lv_obj_set_size(ui_Container1, 580, 220);
    lv_obj_set_flex_flow(ui_Container1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_Container1, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_Container1, lv_color_hex(COL_LEFT_BG), 0);
    lv_obj_set_style_bg_opa(ui_Container1, 255, 0);
    lv_obj_set_style_border_color(ui_Container1, lv_color_hex(COL_BORDER), 0);
    lv_obj_set_style_border_width(ui_Container1, 1, 0);
    lv_obj_set_style_radius(ui_Container1, 6, 0);
    lv_obj_set_style_pad_all(ui_Container1, 16, 0);

    // helper macro for data rows
    #define MAKE_ROW(parent, name_var, name_text, val_var) \
    { \
        lv_obj_t *row = lv_obj_create(parent); \
        lv_obj_remove_style_all(row); \
        lv_obj_set_size(row, 530, 30); \
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW); \
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, \
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); \
        lv_obj_set_style_border_color(row, lv_color_hex(COL_DIVIDER), 0); \
        lv_obj_set_style_border_width(row, 0, 0); \
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0); \
        lv_obj_set_style_pad_bottom(row, 6, 0); \
        lv_obj_t *nm = lv_label_create(row); \
        lv_obj_set_width(nm, 230); \
        lv_label_set_text(nm, name_text); \
        lv_obj_set_style_text_font(nm, &medium1, 0); \
        lv_obj_set_style_text_color(nm, lv_color_hex(COL_DIM), 0); \
        val_var = lv_label_create(row); \
        lv_obj_set_width(val_var, 290); \
        lv_obj_set_style_text_font(val_var, &medium1, 0); \
        lv_obj_set_style_text_color(val_var, lv_color_hex(COL_ACCENT), 0); \
        lv_obj_set_style_text_align(val_var, LV_TEXT_ALIGN_RIGHT, 0); \
    }

    MAKE_ROW(ui_Container1, rpmName, "RPM", rpmLabel)
    lv_label_set_text(rpmLabel, "---- rpm");

    MAKE_ROW(ui_Container1, coolName, "Coolant Temp", ui_Label2)
    lv_label_set_text(ui_Label2, "---.- C");

    MAKE_ROW(ui_Container1, mapName, "Manifold Abs", manifoldPressureLabel)
    lv_label_set_text(manifoldPressureLabel, "waiting");

    MAKE_ROW(ui_Container1, indName, "Indicators", indicatorLabel)
    lv_label_set_text(indicatorLabel, "--");

    MAKE_ROW(ui_Container1, spoilerName, "Spoiler", spoilerLabel)
    lv_label_set_text(spoilerLabel, "---%");

    MAKE_ROW(ui_Container1, canName, "CAN", canStatusLabel)
    lv_label_set_text(canStatusLabel, "starting");

    #undef MAKE_ROW

    // ── Right panel: boost number ─────────────────────────────────
    ui_Container3 = lv_obj_create(ui_Container2);
    lv_obj_remove_style_all(ui_Container3);
    lv_obj_set_size(ui_Container3, 350, 220);
    lv_obj_set_flex_flow(ui_Container3, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_Container3, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_Container3, lv_color_hex(COL_RIGHT_BG), 0);
    lv_obj_set_style_bg_opa(ui_Container3, 255, 0);
    lv_obj_set_style_border_color(ui_Container3, lv_color_hex(COL_BORDER), 0);
    lv_obj_set_style_border_width(ui_Container3, 1, 0);
    lv_obj_set_style_radius(ui_Container3, 6, 0);
    lv_obj_set_style_pad_all(ui_Container3, 10, 0);

    // "BOOST" heading
    lv_obj_t *boostHeading = lv_label_create(ui_Container3);
    lv_obj_set_width(boostHeading, LV_SIZE_CONTENT);
    lv_label_set_text(boostHeading, "BOOST");
    lv_obj_set_style_text_font(boostHeading, &medium1, 0);
    lv_obj_set_style_text_color(boostHeading, lv_color_hex(COL_DIM), 0);
    lv_obj_set_style_text_letter_space(boostHeading, 4, 0);

    ui_Container5 = lv_obj_create(ui_Container3);
    lv_obj_remove_style_all(ui_Container5);
    lv_obj_set_size(ui_Container5, 405, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_Container5, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_Container5, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_Container5, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);

    boostLabel = lv_label_create(ui_Container5);
    lv_obj_set_width(boostLabel, 325);
    lv_obj_set_height(boostLabel, LV_SIZE_CONTENT);
    lv_label_set_text(boostLabel, "----");
    lv_obj_set_style_text_font(boostLabel, &largest1, 0);
    lv_obj_set_style_text_color(boostLabel, lv_color_hex(COL_ACCENT), 0);
    lv_obj_set_style_text_align(boostLabel, LV_TEXT_ALIGN_RIGHT, 0);

    boostUnitLabel = lv_label_create(ui_Container5);
    lv_obj_set_size(boostUnitLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(boostUnitLabel, "kPa");
    lv_obj_set_style_text_font(boostUnitLabel, &medium1, 0);
    lv_obj_set_style_text_color(boostUnitLabel, lv_color_hex(COL_DIM), 0);

    // ── Boost bar ─────────────────────────────────────────────────
    boostBar = lv_bar_create(ui_Panel1);
    lv_obj_set_size(boostBar, 971, 28);
    lv_bar_set_range(boostBar, -100, 200);
    lv_bar_set_value(boostBar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(boostBar, lv_color_hex(COL_BAR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(boostBar, 255, LV_PART_MAIN);
    lv_obj_set_style_border_color(boostBar, lv_color_hex(COL_BORDER),
                                  LV_PART_MAIN);
    lv_obj_set_style_border_width(boostBar, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(boostBar, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(boostBar, lv_color_hex(COL_BAR_FG),
                              LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(boostBar, lv_color_hex(0xFF6600),
                                   LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_dir(boostBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_radius(boostBar, 4, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_color(boostBar, lv_color_hex(COL_ACCENT),
                                  LV_PART_INDICATOR);
    lv_obj_set_style_shadow_width(boostBar, 8, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_opa(boostBar, 120, LV_PART_INDICATOR);
}

void ui_Screen1_screen_destroy(void)
{
    if (ui_Screen1) lv_obj_del(ui_Screen1);

    ui_Screen1       = NULL;
    ui_Panel1        = NULL;
    ui_Container1    = NULL;
    ui_Container2    = NULL;
    ui_Container3    = NULL;
    ui_Container4    = NULL;
    ui_Container5    = NULL;
    ui_Label1        = NULL;
    ui_Label2        = NULL;
    ui_Chart1        = NULL;
    boostLabel       = NULL;
    boostUnitLabel   = NULL;
    oilPressureLabel = NULL;
    manifoldPressureLabel = NULL;
    boostBar         = NULL;
    rpmLabel         = NULL;
    indicatorLabel   = NULL;
    spoilerLabel     = NULL;
    canStatusLabel   = NULL;
}
