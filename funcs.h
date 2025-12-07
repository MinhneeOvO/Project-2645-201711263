#ifndef FUNCS_H
#define FUNCS_H

typedef enum {
    buck_conv = 0,
    boost_conv,
    buck_boost_conv,
    cuk_conv
} converter_type;

//input for all converter
typedef struct {
    converter_type type;
    double vin_min;
    double vin_max;
    double v_out;
    double p_out;
    double f_switch;
    double ripple_i_percent;
    double ripple_i_1_percent;// for cuk converter
    double ripple_i_2_percent;// for cuk converter
    double ripple_v_percent;
    double ripple_v_cn_percent;//for cuk converter
} converter_input;

//output
typedef struct {
    double duty_cycle;
    double r_load;
    double i_out;
    double ripple_i_L;
    double ripple_v_C;
    double L;
    double C;
    double L1;//for cuk converter
    double L2;//for cuk converter
    double Cn;//for cuk converter
    double Co;//for cuk converter
    double i_L_peak;
    double i_LB; //for ccm/dcm
    int is_ccm; // 1 = ccm, 0 = dcm
} converter_result;

void buck_converter(void);
void boost_converter(void);
void buck_boost_converter(void);
void cuk_converter(void);

#endif