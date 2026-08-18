#ifndef UI_SCREEN1_H
#define UI_SCREEN1_H
#ifdef __cplusplus
extern "C" {
#endif

extern lv_obj_t *ui_Screen1;
extern lv_obj_t *boostUnitLabel;
extern lv_obj_t *ui_Panel1;
extern lv_obj_t *ui_Container1;
extern lv_obj_t *ui_Container4;
extern lv_obj_t *ui_Label1;
extern lv_obj_t *ui_Label2;
extern lv_obj_t *ui_Container2;
extern lv_obj_t *ui_Chart1;
extern lv_obj_t *ui_Container3;
extern lv_obj_t *ui_Indicator;
extern lv_obj_t *ui_Container5;
extern lv_obj_t *boostLabel;
extern lv_obj_t *oilPressureLabel;
extern lv_obj_t *manifoldPressureLabel;
extern lv_obj_t *boostBar;
extern lv_obj_t *rpmLabel;
extern lv_obj_t *indicatorLabel;
extern lv_obj_t *spoilerLabel;
extern lv_obj_t *canStatusLabel;

void ui_Screen1_screen_init(void);
void ui_Screen1_screen_destroy(void);

#ifdef __cplusplus
}
#endif
#endif
