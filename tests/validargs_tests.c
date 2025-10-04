#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "const.h"

static char *progname = "bin/birp";

Test(validargs_tests_suite, valid_args_ignore_flags_test, .timeout = 5)
{
    char *argv[] = {progname, "-h", "-x", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int flag = 0x80000000;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d", ret, exp_ret);
    cr_assert_eq(opt & flag, flag, "Correct bit (0x%x) not set for -h. Got: %x", flag, opt);
}

Test(validargs_tests_suite, valid_args_no_parameter_test, .timeout = 5)
{
    char *argv[] = {progname, "-t", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d", ret, exp_ret);
}

Test(validargs_tests_suite, valid_args_incorrect_order_test, .timeout = 5)
{
    char *argv[] = {progname, "-r", "-i", "birp", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d", ret, exp_ret);
}

Test(validargs_tests_suite, valid_args_threshold_bounds_test, .timeout = 5)
{
    char *argv[] = {progname, "-t", "256", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d", ret, exp_ret);
}

Test(validargs_tests_suite, valid_args_zoom_out_test, .timeout = 5)
{
    char *argv[] = {progname, "-i", "birp", "-o", "birp", "-z", "9", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = 0xF70322;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d", ret, exp_ret);
    cr_assert_eq(opt, exp_opt, "Invalid options settings.  Got: 0x%x | Expected: 0x%x", opt, exp_opt);
}

/* Tests given to the students */
Test(validargs_tests_suite, validargs_help_test, .timeout=5) {
    char *argv[] = {progname, "-h", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int flag = 0x80000000;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d",
		 ret, exp_ret);
    cr_assert_eq(opt & flag, flag, "Correct bit (0x%x) not set for -h. Got: %x",
		 flag, opt);
}

Test(validargs_tests_suite, validargs_input_test, .timeout=5) {
    char *argv[] = {progname, "-i", "pgm", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = 0x21;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d",
		 ret, exp_ret);
    cr_assert_eq(opt, exp_opt, "Invalid options settings.  Got: 0x%x | Expected: 0x%x",
		 opt, exp_opt);
}

Test(validargs_tests_suite, validargs_output_test, .timeout=5) {
    char *argv[] = {progname, "-o", "ascii", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = 0x32;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d",
		 ret, exp_ret);
    cr_assert_eq(opt, exp_opt, "Invalid options settings.  Got: 0x%x | Expected: 0x%x",
		 opt, exp_opt);
}

Test(validargs_tests_suite, validargs_transform_test, .timeout=5) {
    char *argv[] = {progname, "-o", "birp", "-i", "birp", "-t", "128", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = 0x800222;
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d",
		 ret, exp_ret);
    cr_assert_eq(opt, exp_opt, "Invalid options settings.  Got: 0x%x | Expected: 0x%x",
		 opt, exp_opt);
}

Test(validargs_tests_suite, validargs_error_test, .timeout=5) {
    char *argv[] = {progname, "-i", "ascii", "-o", "birp", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int exp_ret = -1;
    int ret = validargs(argc, argv);
    cr_assert_eq(ret, exp_ret, "Invalid return for validargs.  Got: %d | Expected: %d",
		 ret, exp_ret);
}

Test(validargs_tests_suite, help_system_test, .timeout=5) {
    char *cmd = "bin/birp -h > /dev/null 2>&1";

    // system is a syscall defined in stdlib.h
    // it takes a shell command as a string and runs it
    // we use WEXITSTATUS to get the return code from the run
    // use 'man 3 system' to find out more
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(invalid_args_tests, comp_trans_combo, .timeout=5){
	char* argv[] = {progname, "-n", "-t", "100", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_ret = -1, opt = 0, flag = 0;
	int ret = validargs(argc,argv);
	cr_assert_eq(ret, exp_ret, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, exp_ret);
	cr_assert_eq(opt, global_options, "Invalid options. Got: 0x%x | Expected: 0x%x", 
			global_options, opt);
}

Test(invalid_args_tests, big_zoomOut_error, .timeout=5){
	char* argv[] = {progname, "-z", "20", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_ret = -1, opt = 0, flag = 0;
	int ret = validargs(argc,argv);
	cr_assert_eq(ret, exp_ret, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, exp_ret);
	cr_assert_eq(opt, global_options, "Invalid options. Got: 0x%x | Expected: 0x%x", 
			global_options, opt);
}


Test(invalid_args_tests, negative_zoomOut_error, .timeout=5){
	char* argv[] = {progname, "-z", "-1", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_ret = -1, opt = 0, flag = 0;
	int ret = validargs(argc,argv);
	cr_assert_eq(ret, exp_ret, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, exp_ret);
	cr_assert_eq(opt, global_options, "Invalid options. Got: 0x%x | Expected: 0x%x", 
			global_options, opt);
}

Test(invalid_args_tests, negative_zoomIn_error, .timeout=5){
	char* argv[] = {progname, "-Z", "-1", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_ret = -1, opt = 0, flag = 0;
	int ret = validargs(argc,argv);
	cr_assert_eq(ret, exp_ret, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, exp_ret);
	cr_assert_eq(opt, global_options, "Invalid options. Got: 0x%x | Expected: 0x%x", 
			global_options, opt);
}

Test(invalid_args_tests, big_zoomIn_error, .timeout=5){
	char* argv[] = {progname, "-Z", "20", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_ret = -1, opt = 0, flag = 0;
	int ret = validargs(argc,argv);
	cr_assert_eq(ret, exp_ret, "Invalid return for validd args. Got: %d | Expected: %d", 
			ret, exp_ret);
	cr_assert_eq(opt, global_options, "Invalid options. Got: 0x%x | Expected: 0x%x", 
			global_options, opt);
}

Test(invalid_args_tests, input_rotation_test, .timeout=5){
	char* argv[] = {progname, "-i" ,  "birp", "-r", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_opt = 0x422; //birp input,birp output, rotation
	int ret = validargs(argc, argv);
	cr_assert_eq(ret, EXIT_SUCCESS, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, EXIT_SUCCESS);
	cr_assert_eq(exp_opt, global_options, "Invalid Global options. Got 0x%x | Expected 0x%x",
			global_options, exp_opt);
}

Test(invalid_args_tests, input_rotation_error, .timeout=5){
	char* argv[] = {progname, "-i" ,  "pgm", "-r", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_opt = 0; //birp input,birp output, rotation
	int ret = validargs(argc, argv);
	cr_assert_eq(ret, -1, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, -1);
	cr_assert_eq(exp_opt, global_options, "Invalid Global options. Got 0x%x | Expected 0x%x",
			global_options, exp_opt);
}


Test(invalid_args_tests, transform_check, .timeout=5){
	char* argv[] = {progname, "-i" ,  "birp", "-t", "34", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_opt = 0x220222;
	int ret = validargs(argc, argv);
	cr_assert_eq(ret, EXIT_SUCCESS, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, EXIT_SUCCESS);
	cr_assert_eq(exp_opt, global_options, "Invalid Global options. Got 0x%x | Expected 0x%x",
			global_options, exp_opt);
}

Test(invalid_args_tests, transform_error, .timeout=5){
	char* argv[] = {progname, "-i" ,  "birp", "-o", "ascii", "-t", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_opt = 0;
	int exp_ret = -1;
	int ret = validargs(argc, argv);
	cr_assert_eq(ret, exp_ret, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, exp_ret);
	cr_assert_eq(exp_opt, global_options, "Invalid Global options. Got 0x%x | Expected 0x%x",
			global_options, exp_opt);

}

Test(invalid_args_tests, out_of_order, .timeout=5){
	char* argv[] = {progname, "-r","-i" ,  "birp", "-o", "ascii", "-t", "34", NULL};
	int argc = (sizeof(argv)/sizeof(char*))-1;
	int exp_opt = 0;
	int ret = validargs(argc, argv);
	cr_assert_eq(ret, -1, "Invalid return for valid args. Got: %d | Expected: %d", 
			ret, -1);
	cr_assert_eq(exp_opt, global_options, "Invalid Global options. Got 0x%x | Expected 0x%x",
			global_options, exp_opt);
}

