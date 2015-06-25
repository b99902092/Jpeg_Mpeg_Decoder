#define picture_start_code    0x00000100
#define slice_start_codes_min 0x00000101
#define slice_start_codes_max 0x000001af
//reserved 000001B0
//reserved 000001B1
#define user_data_start_code  0x000001b2
#define sequence_header_code  0x000001b3
#define sequence_error_code   0x000001b4
#define extension_start_code  0x000001b5
//reserved 000001B6
#define sequence_end_code     0x000001b7
#define group_start_code      0x000001b8
//system start codes(see note) 000001B9 through 000001FF

#define start_code_begin      0x000001

#define pi 3.14159265359
#define e 2.71828182846

#define max_resolution 1024
#define pic_max_size 256