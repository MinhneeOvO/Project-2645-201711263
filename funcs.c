#include <stdio.h>
#include "funcs.h"
#include <math.h> // for function
//gcc main.c funcs.c -o main.exe -lm
//./main.exe
//Read input function
static void read_input(converter_input *input);
//Buck functions
static int  buck_validate_input(const converter_input *input);
static void buck_calculate(const converter_input *input, converter_result *result);
static void buck_analyse(const converter_input *input, converter_result *result);
static void buck_print_result(const converter_input *input, const converter_result *result);
static void buck_save_file(const converter_input *input, const converter_result *result);

//Boost functions
static int  boost_validate_input(const converter_input *input);
static void boost_calculate(const converter_input *input, converter_result *result);
static void boost_analyse(const converter_input *input, converter_result *result);
static void boost_print_result(const converter_input *input, const converter_result *result);
static void boost_save_file(const converter_input *input, const converter_result *result);

//Buck-Boost Converter
static int  buck_boost_validate_input(const converter_input *input);
static void buck_boost_analyse(const converter_input *input, converter_result *result);
static void buck_boost_calculate(const converter_input *input, converter_result *result);
static void buck_boost_print_result(const converter_input *input, const converter_result *result);
static void buck_boost_save_file(const converter_input *input, const converter_result *result);

//Cuk Converter
static int  cuk_validate_input(const converter_input *input);
static void cuk_read_input(converter_input *input);
static void cuk_calculate(const converter_input *input, converter_result *result);
static void cuk_analyse(const converter_input *input, converter_result *result);
static void cuk_print_result(const converter_input *input, const converter_result *result);
static void cuk_save_file(const converter_input *input, const converter_result *result);
//read input

static void read_input(converter_input *input) {
    printf("Enter maximum input voltage: ");
    scanf("%lf", &input->vin_max);
    printf("Enter minimum input voltage: ");
    scanf("%lf", &input->vin_min);
    //vout is negative to vin, the code will handle the negative sign
    printf("Enter output voltage magnitude (positive value): ");
    scanf("%lf", &input->v_out);
    printf("Enter output power: ");
    scanf("%lf", &input->p_out);
    printf("Enter switching frequency: ");
    scanf("%lf", &input->f_switch);
    printf("Enter inductor current ripple (in percent): ");
    scanf("%lf", &input->ripple_i_percent);
    printf("Enter output voltage ripple (%% of Vout): ");
    scanf("%lf", &input->ripple_v_percent);
}
//BUCK CONVERTER
void buck_converter(void) {
    converter_input input = {0};
    converter_result result = {0};
    input.type = buck_conv;
    printf("\n>> Buck Converter\n");
    read_input(&input);
    if (!buck_validate_input(&input)) {
        printf("\nInvalid input\n");
        return;
    }
    buck_calculate(&input, &result);
    buck_analyse(&input, &result);
    buck_print_result(&input, &result);
//ask if user want to save design
    char answer_for_saving;
    printf("\nSave result to file? (y/n): ");
    if (scanf(" %c", &answer_for_saving) == 1 && ((answer_for_saving == 'y') || (answer_for_saving == 'Y'))) {
        buck_save_file(&input, &result);
    }
}

static int buck_validate_input(const converter_input *input) {
    int input_validate = 1;
    //Check if vin and vout are smaller than or equal to 0
    if (input-> vin_min <= 0 || input-> vin_max <= 0 || input->v_out <= 0) {
        printf("ERROR: Voltages must be > 0! \n");
        input_validate = 0;
    }
    //Check if vin_min > vin_max
    if (input-> vin_min > input-> vin_max) {
        printf("ERROR: minimum voltage must be smaller than maximum voltage!\n");
        input_validate = 0;
    }
    //Check if vin_min < v_out. Buck is a stepdown converter
    if (input-> v_out >= input->vin_min) {
        printf("ERROR: Buck converter is step-down, require output voltage < minimum input voltage! \n");
        input_validate = 0;
    }
    //Check if p_out < 0
    if (input->p_out <= 0) {
        printf("ERROR: Output power must be larger than 0! \n");
        input_validate = 0;
    }
    //Check if switching frequency is smaller than 0
    if (input-> f_switch <= 0) {
        printf("ERROR: Switching frequency must be larger than 0! \n");
        input_validate = 0;
    }
    //Check if current ripple is in range 0 - 100
    if (input->ripple_i_percent <= 0 || input->ripple_i_percent > 100) {
        printf("ERROR: Inductor current ripple percentage must be > 0 and < 100. \n");
        input_validate = 0;
    }
    if (input->ripple_v_percent <= 0 || input->ripple_v_percent > 100) {
        printf("ERROR: Voltage ripple percentage must be > 0 and < 100. \n ");
        input_validate = 0;
    }
    return input_validate;
}

static void buck_calculate(const converter_input *input, converter_result *result) {
    //use vin_min for worst case
    double vin_worst = input->vin_min;
    // K=Vout/Vin
    result->duty_cycle = input->v_out / vin_worst;
    //Rload from P-out
    result->r_load = (input->v_out * input->v_out)/input->p_out;
    //I out = Vout/Rload
    result->i_out = input->v_out/result->r_load;
    //From rippe percent to 0. Use 100.0 because use double
    result->ripple_i_L = (input->ripple_i_percent / 100.0)*result->i_out;
    //Inductor use vin max for worst case. Equation from 2501
    double vin_L = input->vin_max;
    result->L = (vin_L - input-> v_out)*result->duty_cycle/(input->f_switch*result->ripple_i_L);
    //Capacitance. Equation from 2501
    result->ripple_v_C = (input->ripple_v_percent/100.0)*input->v_out;
    result->C = (result->ripple_i_L/(8.0*input->f_switch*result->ripple_v_C));
}

static void buck_analyse(const converter_input * input, converter_result *result) {
    //Check boundary condition dcm and ccm using i LB
    result->i_LB = result-> ripple_i_L/2.0;
    //Check ccm 1 or 0
    if (result->i_out > result->i_LB) {
    result->is_ccm = 1;
    }
    else {result->is_ccm = 0;}
    //i_L peak
    result->i_L_peak = result->i_out + result->ripple_i_L/2.0;
    //Warning for DCM, high ripple current and volatge. Industry use.
    if (!result->is_ccm) {
        printf("WARNING: Converter is in DCM\n");
    }
    if (result->ripple_i_L > 0.4*result->i_out) {
        printf("WARNING: Inductor ripple > 40%% of I out, consider using higher inductance inductor.\n");
    }
    if (input->ripple_v_percent > 5.0) {
        printf("WARNING: Voltage ripple > 5%% of V out, consider using higher capacitance capacitor.\n");
    }
}

static void buck_print_result(const converter_input *input, const converter_result *result) {
    printf("\n========== BUCK CONVERTER DESIGN ==========\n");
    //Input data
    printf("Input data:\n");
    printf("Minimum input voltage      (Vin min)  = %.2f V\n", input->vin_min);
    printf("Maximum input voltage      (Vin max)  = %.2f V\n", input->vin_max);
    printf("Output voltage             (Vout)     = %.2f V\n", input->v_out);
    printf("Output power               (Pout)     = %.2f W\n", input->p_out);
    printf("Switching frequency        (fs)       = %.0f Hz\n", input->f_switch);
    printf("Current ripple                        = %.1f %% of Iout\n", input->ripple_i_percent);
    printf("Voltage ripple                        = %.1f %% of Vout\n\n", input->ripple_v_percent);
    //Output data. Using symbols for easier read.
    printf("Requirement device and output data:\n");
    printf("Duty cycle                 (K)        = %.2f\n", result->duty_cycle);
    printf("Resistor load              (Rload)    = %.2f Ohms\n", result->r_load);
    printf("Output current             (Iout)     = %.2f A\n", result->i_out);
    printf("Inductor current ripple    (delta IL) = %.3f A\n", result->ripple_i_L);
    printf("Inductor                   (L)        = %.6e H\n", result->L);
    printf("Capacitor                  (C)        = %.6e F\n", result->C);
    printf("Inductor current peak      (IL peak)  = %.3f A\n", result->i_L_peak);
    printf("Boundary inductor currnet  (ILB)      = %.3f A\n", result->i_LB);
    if (result->is_ccm) {printf("Mode                                  = CCM\n");}
    else {printf("Mode                                  = DCM\n");}
}

static void buck_save_file(const converter_input *input, const converter_result *result) {
    FILE *file_open = fopen("buck_results.txt", "a");
    
    if (!file_open) {perror("fopen");
        return;
    }
    char *mode; // Pointer is applied here
    if (result->is_ccm) {mode = "CCM";}
    else {mode = "DCM";}
    fprintf(file_open, "BUCK, Vin_min=%.3f, Vin_max=%.3f, Vout=%.3f, Pout=%.3f, f_sw=%.0f, "
            "L=%.6e, C=%.6e, R_load=%.3f, Iout=%.3f, mode=%s\n", input->vin_min, input->vin_max, input->v_out, input->p_out, input->f_switch,
            result->L, result->C, result->r_load, result->i_out,
            mode);
    fclose(file_open);
    printf("Results saved to buck_results.txt\n");
}

//Boost converter
void boost_converter(void) {
    converter_input input = {0};
    converter_result result = {0};
    
    input.type = boost_conv;
    printf("\n>> Boost Converter\n");
    read_input(&input);
    if (!boost_validate_input(&input)) {
        printf("Invalid input\n");
        return;
    }
    boost_calculate(&input, &result);
    boost_analyse(&input, &result);
    boost_print_result(&input, &result);
    //Ask if user want to save result
    char answer_for_saving;
    printf("\nSave result to file? (y/n):");
    if (scanf(" %c", &answer_for_saving)==1 && (answer_for_saving == 'y' || answer_for_saving == 'Y')) {
        boost_save_file(&input, &result);
    }
}

static int boost_validate_input(const converter_input *input) {
    int input_validate = 1;

    if (input->vin_min <= 0 || input->vin_max <= 0 || input->v_out <= 0) {
        printf("ERROR: Voltages must be > 0!\n");
        input_validate = 0;
    }
    
    if (input->vin_min > input->vin_max) {
        printf("ERROR: Minimum input voltage must be <= maximum input voltage!\n");
        input_validate = 0;
    }
    // Boost is step-up: Vout > Vin_max
    if (input->v_out <= input->vin_max) {
        printf("ERROR: Boost converter is step-up, require Vout > Vin_max!\n");
        input_validate = 0;
    }
    if (input->p_out <= 0) {
        printf("ERROR: Output power must be larger than 0!\n");
        input_validate = 0;
    }
    if (input->f_switch <= 0) {
        printf("ERROR: Switching frequency must be larger than 0!\n");
        input_validate = 0;
    }
    if (input->ripple_i_percent <= 0 || input->ripple_i_percent > 100) {
        printf("ERROR: Inductor current ripple percentage must be in > 0 and < 100.\n");
        input_validate = 0;
    }
    if (input->ripple_v_percent <= 0 || input->ripple_v_percent > 100) {
        printf("ERROR: Voltage ripple percentage must be > 0 and < 100.\n");
        input_validate = 0;
    }

    return input_validate;
}

static void boost_calculate(const converter_input *input, converter_result *result) {
    //Equation from ELEC2501
    //Vin min for wrost case
    result->duty_cycle = 1.0 - input->vin_min/input->v_out;
    result->r_load = (input->v_out*input->v_out)/input->p_out;
    result->i_out = input->p_out/input->v_out;
    double i_l_avg = input->p_out/input->vin_min;
    result->ripple_i_L = input->ripple_i_percent*i_l_avg/100.0;
    result->L = input->vin_min*result->duty_cycle/(result->ripple_i_L*input->f_switch);
    result->ripple_v_C = (input->ripple_v_percent/100.0)*input->v_out;
    result->C = (result->i_out*result->duty_cycle)/(input->f_switch*result->ripple_v_C);
}

static void boost_analyse(const converter_input *input, converter_result *result) {
    double i_L = input->p_out/input->vin_min;
    result->i_LB =result->ripple_i_L/2.0;
    if (i_L > result->i_LB) {
        result->is_ccm = 1;
    }
    else {result->is_ccm = 0;}
    result->i_L_peak = i_L + result->ripple_i_L/2.0;
    if (!result->is_ccm) {
        printf("WARNING: Converter is in DCM at rated load (IL_avg <= IL/2).\n");
    }
    if (input->ripple_i_percent > 40) {
        printf("WARNING: Inductor ripple in percent > 40%%, consider increasing L.\n");
    }
    if (input->ripple_v_percent > 5.0) {
        printf("WARNING: Voltage ripple > 5%% of Vout, consider increasing C.\n");
    }
}

static void boost_print_result(const converter_input *input, const converter_result *result) {
    double i_in = input->p_out / input->vin_min;

    printf("\n========== BOOST CONVERTER DESIGN ==========\n");
    printf("Input data:\n");
    printf("Minimum input voltage      (Vin min)  = %.2f V\n", input->vin_min);
    printf("Maximum input voltage      (Vin max)  = %.2f V\n", input->vin_max);
    printf("Output voltage             (Vout)     = %.2f V\n", input->v_out);
    printf("Output power               (Pout)     = %.2f W\n", input->p_out);
    printf("Switching frequency        (fs)       = %.0f Hz\n", input->f_switch);
    printf("Inductor ripple                       = %.1f %% of IL\n", input->ripple_i_percent);
    printf("Voltage ripple                        = %.1f %% of Vout\n\n", input->ripple_v_percent);

    printf("Requirement device and output data:\n");
    printf("Duty cycle                 (D)        = %.3f\n", result->duty_cycle);
    printf("Load resistance            (Rload)    = %.2f Ohms\n", result->r_load);
    printf("Output current             (Iout)     = %.3f A\n", result->i_out);
    printf("Input current (IL avg)     (Iin)      = %.3f A\n", i_in);
    printf("Inductor current ripple    (delta IL) = %.3f A\n", result->ripple_i_L);
    printf("Inductor                   (L)        = %.6e H\n", result->L);
    printf("Capacitor                  (C)        = %.6e F\n", result->C);
    printf("Inductor peak current      (IL peak)  = %.3f A\n", result->i_L_peak);
    printf("Boundary inductor current  (ILB)      = %.3f A\n", result->i_LB);
    if (result->is_ccm) {
        printf("Mode                                  = CCM\n");
    }
    else {
        printf("Mode                                  = DCM\n");
    }
}

static void boost_save_file(const converter_input *input, const converter_result *result) {
    FILE *fp = fopen("boost_results.txt", "a");
    if (!fp) {
        perror("fopen");
        return;
    }
    double i_in = input->p_out / input->vin_min;
    char *mode; // Pointer is applied here
    if (result->is_ccm) {mode = "CCM";}
    else {mode = "DCM";}
    fprintf(fp,
            "BOOST, Vin_min=%.3f, Vin_max=%.3f, Vout=%.3f, Pout=%.3f, f_sw=%.0f, "
            "L=%.6e, C=%.6e, R_load=%.3f, Iout=%.3f, Iin=%.3f, mode=%s\n",
            input->vin_min, input->vin_max, input->v_out, input->p_out, input->f_switch,
            result->L, result->C, result->r_load, result->i_out, i_in,
            mode);
    fclose(fp);
    printf("Results saved to boost_results.txt\n");
}

//Buck_Boost Converter     
void buck_boost_converter(void) {
    printf("\n>> Buck-Boost Converter\n");
    converter_input input = {0};
    converter_result result = {0};
    read_input(&input);
    input.type = buck_boost_conv;
    if (!buck_boost_validate_input(&input)) {
        printf("\nInvalid input\n");
        return;
    }
    buck_boost_calculate(&input, &result);
    buck_boost_analyse(&input, &result);
    buck_boost_print_result(&input, &result);
    char answer_for_saving;
    printf("\nSave result to file? (y/n): ");
    if (scanf(" %c", &answer_for_saving) == 1 && ((answer_for_saving == 'y') || (answer_for_saving == 'Y'))) {
        buck_boost_save_file(&input, &result);
    }
    /* you can call a function from here that handles menu 3 */
}

static int buck_boost_validate_input(const converter_input *input) {
    int validate_input = 1;
    //buck_boost converter can be step up or step down converter so no need to check relation between vin and vout
    if (input->vin_min <= 0 || input->vin_max <= 0 || input->v_out <= 0) {
        printf("ERROR: Voltages must be > 0!\n");
        validate_input = 0;
    }
    if (input->vin_min > input->vin_max) {
        printf("ERROR: Vin_min must be <= Vin_max!\n");
        validate_input = 0;
    }
    if (input->p_out <= 0) {
        printf("ERROR: Output power must be > 0!\n");
        validate_input = 0;
    }
    if (input->f_switch <= 0) {
        printf("ERROR: Switching frequency must be > 0!\n");
        validate_input = 0;
    }
    if (input->ripple_i_percent <= 0 || input->ripple_i_percent > 100) {
        printf("ERROR: Ripple current percent must be > 0 and < 100.\n");
        validate_input = 0;
    }
    if (input->ripple_v_percent <= 0 || input->ripple_v_percent > 100) {
        printf("ERROR: Ripple voltage percent must be > 0 and < 100.\n");
        validate_input = 0;
    }
    return validate_input;
}

static void buck_boost_calculate(const converter_input *input, converter_result *result) {
    //vin min for worst case
    //Equation from 2501
    result->duty_cycle = input->v_out/(input->vin_min+input->v_out);
    result->i_out = input->p_out/input->v_out;
    result->r_load = (input->v_out * input->v_out)/input->p_out;
    double i_L = result->i_out/(1.0-result->duty_cycle);
    result->ripple_i_L = (input->ripple_i_percent/100.0)*i_L;
    result->L = input->vin_min*result->duty_cycle/(input->f_switch*result->ripple_i_L);
    result->ripple_v_C = (input->ripple_v_percent/100.0)*input->v_out;
    result->C = result->i_out*result->duty_cycle/(result->ripple_v_C*input->f_switch);
}

static void buck_boost_analyse(const converter_input *input, converter_result *result) {
    double i_L = result->i_out / (1.0 - result->duty_cycle);
    result->i_LB = result->ripple_i_L/2.0;
    result->i_L_peak = i_L + result->ripple_i_L / 2.0;
    if (i_L > result->i_LB) {
        result->is_ccm = 1;
    }
    else {result->is_ccm = 0;}
    
    if (!result->is_ccm) {
        printf("WARNING: Converter is in DCM at rated load.\n");
    }
    if (input->ripple_i_percent > 40.0) {
        printf("WARNING: Inductor ripple > 40%% of IL, consider increasing L.\n");
    }
    if (input->ripple_v_percent > 5.0) {
        printf("WARNING: Voltage ripple > 5%% of |Vout|, consider increasing C.\n");
    }
}

static void buck_boost_print_result(const converter_input *input, const converter_result *result) {
    double i_L = result->i_out / (1.0 - result->duty_cycle);
    printf("\n========== BUCK-BOOST CONVERTER DESIGN ==========\n");
    printf("Input data:\n");
    printf("Minimum input voltage      (Vin min)  = %.2f V\n", input->vin_min);
    printf("Maximum input voltage      (Vin max)  = %.2f V\n", input->vin_max);
    printf("Output voltage magnitude   (|Vout|)   = %.2f V\n", input->v_out);
    printf("Note: Actual Vout is negative (inverting topology).\n");
    printf("Output power               (Pout)     = %.2f W\n", input->p_out);
    printf("Switching frequency        (fs)       = %.0f Hz\n", input->f_switch);
    printf("Inductor ripple                       = %.1f %% of IL\n", input->ripple_i_percent);
    printf("Voltage ripple                        = %.1f %% of |Vout|\n\n", input->ripple_v_percent);

    printf("Requirement device and output data:\n");
    printf("Duty cycle                 (D)        = %.3f\n", result->duty_cycle);
    printf("Load resistance            (Rload)    = %.3f Ohms\n", result->r_load);
    printf("Output current             (Iout)     = %.3f A\n", result->i_out);
    printf("Inductor current           (IL)       = %.3f A\n", i_L);
    printf("Inductor current ripple    (delta IL) = %.3f A\n", result->ripple_i_L);
    printf("Inductor                   (L)        = %.6e H\n", result->L);
    printf("Capacitor                  (C)        = %.6e F\n", result->C);
    printf("Inductor peak current      (IL peak)  = %.3f A\n", result->i_L_peak);
    printf("Boundary inductor current  (ILB)      = %.3f A\n", result->i_LB);
    if (result->is_ccm) {printf("Mode                                  = CCM\n");}
    else {printf("Mode                                  = DCM\n");} 
}

static void buck_boost_save_file(const converter_input *input, const converter_result *result) {
    FILE *fp = fopen("buck_boost_results.txt", "a");
    if (!fp) {
        perror("fopen");
        return;
    }

    double i_L = result->i_out / (1.0 - result->duty_cycle);
    char *mode; // Pointer is applied here
    if (result->is_ccm) {mode = "CCM";}
    else {mode = "DCM";}

    fprintf(fp,
            "BUCK-BOOST, Vin_min=%.3f, Vin_max=%.3f, |Vout|=%.3f, Pout=%.3f, f_sw=%.0f, "
            "L=%.6e, C=%.6e, R_load=%.3f, Iout=%.3f, IL_avg=%.3f, mode=%s\n",
            input->vin_min, input->vin_max, input->v_out, input->p_out, input->f_switch,
            result->L, result->C, result->r_load, result->i_out, i_L, mode);

    fclose(fp);
    printf("Results saved to buck_boost_results.txt\n");
}

//CUK CONVERTER
void cuk_converter(void) {
    printf("\n>> Cuk Converter\n");
    converter_input input = {0};
    converter_result result = {0};
    cuk_read_input(&input);
    input.type = cuk_conv;
    if (!cuk_validate_input(&input)) {
        printf("\nInvalid input\n");
        return;
    }
    cuk_calculate(&input, &result);
    cuk_analyse(&input, &result);
    cuk_print_result(&input, &result);
    char answer_for_saving;
    printf("\nSave result to file? (y/n): ");
    if (scanf(" %c", &answer_for_saving) == 1 && ((answer_for_saving == 'y') || (answer_for_saving == 'Y'))) {
        cuk_save_file(&input, &result);
    }
    /* you can call a function from here that handles menu 4 */
}

static void cuk_read_input(converter_input *input) {
    printf("Enter maximum input voltage: ");
    scanf("%lf", &input->vin_max);
    printf("Enter minimum input voltage: ");
    scanf("%lf", &input->vin_min);
    //vout is negative to vin, the code will handle the negative sign
    printf("Enter output voltage magnitude (positive value): ");
    scanf("%lf", &input->v_out);
    printf("Enter output power: ");
    scanf("%lf", &input->p_out);
    printf("Enter switching frequency: ");
    scanf("%lf", &input->f_switch);
    printf("Enter first inductor current ripple (%% of IL): ");
    scanf("%lf", &input->ripple_i_1_percent);
    printf("Enter second inductor current ripple (%% of IL): ");
    scanf("%lf", &input->ripple_i_2_percent);
    printf("Enter output voltage ripple (%% of Vout): ");
    scanf("%lf", &input->ripple_v_percent);
    printf("Enter Cn voltage ripple (%% of Vin): ");
    scanf("%lf", &input->ripple_v_cn_percent);
}

static void cuk_calculate(const converter_input *input, converter_result *result) {
    //vin_min for wrost case
    //equation from 2501
    result->duty_cycle = input->v_out/(input->vin_min + input->v_out);
    result->r_load = (input->v_out*input->v_out)/input->p_out;
    result->i_out = input->p_out/input->v_out;
    double i_in = input->p_out / input->vin_min;
    double delta_IL_1 = i_in*input->ripple_i_1_percent/100.0;
    double delta_IL_2 = result->i_out*input->ripple_i_2_percent/100.0;
    double delta_v_out = input->v_out*input->ripple_v_percent/100.0;
    double delta_v_cn = input->vin_min*input->ripple_v_cn_percent/100.0;
    result->L1 = (input->vin_min*result->duty_cycle)/(input->f_switch*delta_IL_1);
    result->L2 = (input->v_out*(1.0-result->duty_cycle))/(input->f_switch*delta_IL_2);
    result->Co = input->v_out*(1.0-result->duty_cycle)/(8.0*input->f_switch*input->f_switch*delta_v_out*result->L2);
    result->Cn = (result->i_out*(1.0-result->duty_cycle))/(input->f_switch*delta_v_cn);
}

static int cuk_validate_input(const converter_input *input) {
    int input_validate = 1;

    if (input->vin_min <= 0 || input->vin_max <= 0 || input->v_out <= 0) {
        printf("ERROR: Voltages must be > 0!\n");
        input_validate = 0;
    }

    if (input->vin_min > input->vin_max) {
        printf("ERROR: Minimum input voltage must be smaller than or equal to maximum input voltage!\n");
        input_validate = 0;
    }

    if (input->p_out <= 0) {
        printf("ERROR: Output power must be larger than 0!\n");
        input_validate = 0;
    }

    if (input->f_switch <= 0) {
        printf("ERROR: Switching frequency must be larger than 0!\n");
        input_validate = 0;
    }

    //L1: 0–100%
    if (input->ripple_i_1_percent <= 0 || input->ripple_i_1_percent > 100) {
        printf("ERROR: L1 current ripple percentage must be > 0 and < 100.\n");
        input_validate = 0;
    }

    // L2: 0–100%
    if (input->ripple_i_2_percent <= 0 || input->ripple_i_2_percent > 100) {
        printf("ERROR: L2 current ripple percentage must be > 0 and < 100.\n");
        input_validate = 0;
    }

    //Co: 0–100%
    if (input->ripple_v_percent <= 0 || input->ripple_v_percent > 100) {
        printf("ERROR: Output voltage ripple percentage must be > 0 and < 100.\n");
        input_validate = 0;
    }

    // Cn: 0–100%
    if (input->ripple_v_cn_percent <= 0 || input->ripple_v_cn_percent > 100) {
        printf("ERROR: Cn voltage ripple percentage must be > 0 and < 100.\n");
        input_validate = 0;
    }

    return input_validate;
}

static void cuk_analyse(const converter_input *input, converter_result *result) {
    //calcaulate worst delta IL and IL to detect ccm or dcm
    double i_in = input->p_out/input->vin_min;
    double delta_IL_1 = i_in*input->ripple_i_1_percent/100.0;
    double delta_IL_2 = result->i_out*input->ripple_i_2_percent/100.0;
    double worst_delta_IL;
    double worst_IL;
    if (delta_IL_1 > delta_IL_2) {
        worst_delta_IL = delta_IL_1;
    }
    else {worst_delta_IL = delta_IL_2;}

    if (i_in>result->i_out) {
        worst_IL = i_in;
    }
    else {worst_IL = result->i_out;}
    result->i_LB = worst_delta_IL/2.0;
    result->i_L_peak = worst_IL + worst_delta_IL/2.0;
    //ccm or dcm
    if (worst_IL > result->i_LB) {
        result->is_ccm = 1;
    }
    else {result->is_ccm = 0;}

    if (!result->is_ccm) {
        printf("WARNING: Cuk converter may operate in DCM at rated load (IL <= IL/2).\n");
    }

    if (input->ripple_i_1_percent > 40.0 || input->ripple_i_2_percent > 40.0) {
        printf("WARNING: Inductor current ripple > 40%% of average for at least one inductor; consider increasing L1 and/or L2.\n");
    }

    if (input->ripple_v_percent > 5.0) {
        printf("WARNING: Output voltage ripple > 5%% of |Vout|; consider increasing Co.\n");
    }

    if (input->ripple_v_cn_percent > 10.0) {
        printf("WARNING: Cn voltage ripple > 10%% of Vin; consider increasing Cn.\n");
    }
}

static void cuk_print_result(const converter_input *input,const converter_result *result) {

    double vout_mag   = input->v_out;                 // |Vout|
    double i_in       = input->p_out / input->vin_min; // IL1_avg
    double i_L1_avg   = i_in;
    double i_L2_avg   = result->i_out;                // ≈ |Iout|
    double delta_IL_1 = i_L1_avg * input->ripple_i_1_percent / 100.0;
    double delta_IL_2 = i_L2_avg * input->ripple_i_2_percent / 100.0;

    printf("\n========== CUK CONVERTER DESIGN ==========\n");

    //Input data
    printf("Input data:\n");
    printf("Minimum input voltage      (Vin min)    = %.2f V\n", input->vin_min);
    printf("Maximum input voltage      (Vin max)    = %.2f V\n", input->vin_max);
    printf("Output voltage magnitude   (|Vout|)     = %.2f V\n", vout_mag);
    printf("NOTE: Actual output voltage is negative (inverting topology).\n");
    printf("Output power               (Pout)       = %.2f W\n", input->p_out);
    printf("Switching frequency        (fs)         = %.0f Hz\n", input->f_switch);
    printf("L1 current ripple                       = %.1f %% of IL1\n",input->ripple_i_1_percent);
    printf("L2 current ripple                       = %.1f %% of IL2\n",input->ripple_i_2_percent);
    printf("Output voltage ripple                   = %.1f %% of |Vout|\n",input->ripple_v_percent);
    printf("Coupling-capacitor voltage ripple       = %.1f %% of Vin\n\n",input->ripple_v_cn_percent);

    //Output/required component values
    printf("Required device values and key results:\n");
    printf("Duty cycle                 (D)          = %.3f\n", result->duty_cycle);
    printf("Load resistance            (Rload)      = %.3f Ohms\n", result->r_load);
    printf("Output current magnitude   (|Iout|)     = %.3f A\n", result->i_out);

    printf("Input inductor avg current (IL1 avg)    = %.3f A\n", i_L1_avg);
    printf("Output inductor avg current(IL2 avg)    = %.3f A\n", i_L2_avg);
    printf("L1 current ripple          (delta IL1)  = %.3f A\n", delta_IL_1);
    printf("L2 current ripple          (delta IL2)  = %.3f A\n\n", delta_IL_2);

    printf("Inductor L1                (L1)         = %.6e H\n", result->L1);
    printf("Inductor L2                (L2)         = %.6e H\n", result->L2);
    printf("Output capacitor           (Co)         = %.6e F\n", result->Co);
    printf("Coupling capacitor         (Cn)         = %.6e F\n\n", result->Cn);

    printf("Worst-case inductor peak   (IL_peak)    = %.3f A\n", result->i_L_peak);
    printf("Boundary inductor current  (ILB)        = %.3f A\n", result->i_LB);

    if (result->is_ccm) {
    printf("Mode                                    = CCM\n");
    } else {
    printf("Mode                                    = DCM\n");
    }
}

static void cuk_save_file(const converter_input *input, const converter_result *result) {
    FILE *fp = fopen("cuk_results.txt", "a");
    if (!fp) {
        perror("fopen");
        return;
    }

    double i_in       = input->p_out / input->vin_min;     // IL1_avg
    double i_L1_avg   = i_in;
    double i_L2_avg   = result->i_out;                     // |Iout|
    double delta_IL_1 = i_L1_avg * input->ripple_i_1_percent / 100.0;
    double delta_IL_2 = i_L2_avg * input->ripple_i_2_percent / 100.0;
    char *mode; // Pointer is applied here
    if (result->is_ccm) {mode = "CCM";}
    else {mode = "DCM";}

    fprintf(fp,"CUK, Vin_min=%.3f, Vin_max=%.3f, |Vout|=%.3f, Pout=%.3f, f_sw=%.0f, "
    "L1=%.6e, L2=%.6e, Co=%.6e, Cn=%.6e, "
    "IL1_avg=%.3f, IL2_avg=%.3f, delta IL1=%.3f, delta IL2=%.3f, "
    "IL_peak=%.3f, ILB=%.3f, mode=%s\n",
    input->vin_min, input->vin_max, input->v_out, input->p_out, input->f_switch,
    result->L1, result->L2, result->Co, result->Cn,
    i_L1_avg, i_L2_avg, delta_IL_1, delta_IL_2,
    result->i_L_peak, result->i_LB, mode);
    fclose(fp);
    printf("Results saved to cuk_results.txt\n");
}