
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#include "utils.h"
#include "define.h"
#include "TableHandler.h"
#include "idct.h"

#include <stdio.h>

#define sign(integer) (integer > 0 ? 1 : (integer < 0 ? -1 : 0))

typedef struct sequence_header{
  int horizontal_size;
  int vertical_size;
  int pel_aspect_ratio;
  int picture_rate;
  int bit_rate;
  bool marker_bit;
  int vbv_buffer_size;
  bool constrained_parameter_flag;
  bool load_intra_quantizer_matrix;
  int intra_quantizer_matrix[64];
  bool load_non_intra_quantizer_matrix;
  int non_intra_quantizer_matrix[64];
  int sequence_extension_data;
  int user_data;

  int mb_height;
  int mb_width;
  double fps;
} Sequence_header;

typedef struct group_header{
  int time_code;
  bool closed_gop;
  bool broken_link;
  int group_extension_data;
  int user_data;

  int current_image_ycbcr[3][max_resolution][max_resolution];
  int forward_image_ycbcr[3][max_resolution][max_resolution];
  int backward_image_ycbcr[3][max_resolution][max_resolution];
} Group_header;

typedef struct picture_header{
  int temporal_reference;
  int picture_coding_type;
  int vbv_delay;
  bool full_pel_forward_vector;
  int forward_f_code;
  int forward_r_size;
  int forward_f;
  bool full_pel_backward_vector;
  int backward_f_code;
  int backward_r_size;
  int backward_f;
  bool extra_bit_picture;
  int extra_information_picture;
  int picture_extension_data;
  int user_data;

} Picture_header;

typedef struct slice_header{
  int quantizer_scale;
  bool extra_bit_slice;
  int extra_information_slice;

  int past_intra_address;
  int dct_dc_y_past;
  int dct_dc_cb_past;
  int dct_dc_cr_past;
} Slice_header;

typedef struct macroblck_header{
  int macroblock_stuffing;
  int macroblock_escape;
  int macroblock_address_increment;
  int macroblock_type;
  int motion_horizontal_forward_code;
  int motion_horizontal_forward_r;
  int motion_vertical_forward_code;
  int motion_vertical_forward_r;
  int motion_horizontal_backward_code;
  int motion_horizontal_backward_r;
  int motion_vertical_backward_code;
  int motion_vertical_backward_r;
  int coded_block_pattern;
  bool end_of_macroblock;

  int mb_row;
  int mb_col;
  int macroblock_address;

  bool macroblock_quant;
  bool macroblock_motion_forward;
  bool macroblock_motion_backward;
  bool macroblock_pattern;
  bool macroblock_intra;
} Macroblock;

typedef struct block_header{
  int dct_dc_size_luminance;
  int dct_dc_differential;
  int dct_dc_size_chrominance;
  int dct_coeff_first;
  int dct_coeff_next;
  int end_of_block;
} Block;

Sequence_header sequence;
Group_header group;
Picture_header picture;
Slice_header slice;
Macroblock macroblock;
Block block;

int haha = 0, qq = 0;
int dct_zz[64];
int dct_recon[8][8];
int pel_for[8][8], pel_back[8][8], pel_avg[8][8];

int previous_macroblock_address;
int pattern_code[6];

int recon[4], recon_prev[4];

int macroblock_previous_motion_forward, macroblock_previous_motion_backward;

Mat pic_list[pic_max_size];
int pic_index = 0;

int bound(double v){
  if (v>255)
    return 255;
  else if (v < 0)
    return 0;
  else
    return v;
}

void forward_motion_prediction(){
  int complement_horizontal_forward_r;
  int complement_vertical_forward_r;
  int right_little, right_big, down_little, down_big;

  if (picture.forward_f == 1 || macroblock.motion_horizontal_forward_code == 0){
    complement_horizontal_forward_r = 0;
  }
  else{
    complement_horizontal_forward_r = picture.forward_f - 1 - macroblock.motion_horizontal_forward_r;
  }

  if (picture.forward_f == 1 || macroblock.motion_vertical_forward_code == 0){
    complement_vertical_forward_r = 0;
  }
  else{
    complement_vertical_forward_r = picture.forward_f - 1 - macroblock.motion_vertical_forward_r;
  }

  right_little = macroblock.motion_horizontal_forward_code * picture.forward_f;
  if (right_little == 0){
    right_big = 0;
  }
  else{
    if (right_little>0){
      right_little -= complement_horizontal_forward_r;
      right_big = right_little - 32 * picture.forward_f;
    }
    else{
      right_little += complement_horizontal_forward_r;
      right_big = right_little + 32 * picture.forward_f;
    }
  }

  down_little = macroblock.motion_vertical_forward_code * picture.forward_f;
  if (down_little == 0){
    down_big = 0;
  }
  else{
    if (down_little>0){
      down_little = down_little - complement_vertical_forward_r;
      down_big = down_little - 32 * picture.forward_f;
    }
    else{
      down_little = down_little + complement_vertical_forward_r;
      down_big = down_little + 32 * picture.forward_f;
    }
  }

  int max = (16 * picture.forward_f) - 1;
  int min = (-16 * picture.forward_f);
  int new_vector = recon_prev[0] + right_little;

  if (new_vector <= max && new_vector >= min)
    recon[0] = recon_prev[0] + right_little;
  else
    recon[0] = recon_prev[0] + right_big;

  recon_prev[0] = recon[0];
  if (picture.full_pel_forward_vector) 
    recon[0] = recon[0] << 1;

  new_vector = recon_prev[1] + down_little;
  if (new_vector <= max && new_vector >= min)
    recon[1] = recon_prev[1] + down_little;
  else
    recon[1] = recon_prev[1] + down_big;
  recon_prev[1] = recon[1];
  if (picture.full_pel_forward_vector)
    recon[1] = recon[1] << 1;
}

void backward_motion_prediction(){
  int complement_horizontal_backward_r;
  int complement_vertical_backward_r;
  int right_little, right_big, down_little, down_big;

  if (picture.backward_f == 1 || macroblock.motion_horizontal_backward_code == 0){
    complement_horizontal_backward_r = 0;
  }
  else{
    complement_horizontal_backward_r = picture.backward_f - 1 - macroblock.motion_horizontal_backward_r;
  }

  if (picture.backward_f == 1 || macroblock.motion_vertical_backward_code == 0){
    complement_vertical_backward_r = 0;
  }
  else{
    complement_vertical_backward_r = picture.backward_f - 1 - macroblock.motion_vertical_backward_r;
  }

  right_little = macroblock.motion_horizontal_backward_code * picture.backward_f;
  if (right_little == 0){
    right_big = 0;
  }
  else{
    if (right_little>0){
      right_little -= complement_horizontal_backward_r;
      right_big = right_little - 32 * picture.backward_f;
    }
    else{
      right_little += complement_horizontal_backward_r;
      right_big = right_little + 32 * picture.backward_f;
    }
  }

  down_little = macroblock.motion_vertical_backward_code * picture.backward_f;
  if (down_little == 0){
    down_big = 0;
  }
  else{
    if (down_little>0){
      down_little = down_little - complement_vertical_backward_r;
      down_big = down_little - 32 * picture.backward_f;
    }
    else{
      down_little = down_little + complement_vertical_backward_r;
      down_big = down_little + 32 * picture.backward_f;
    }
  }

  int max = (16 * picture.backward_f) - 1;
  int min = (-16 * picture.backward_f);

  int new_vector = recon_prev[2] + right_little;
  if (new_vector <= max && new_vector >= min)
    recon[2] = recon_prev[2] + right_little;
  else
    recon[2] = recon_prev[2] + right_big;
  recon_prev[2] = recon[2];
  if (picture.full_pel_backward_vector) recon[2] = recon[2] << 1;

  new_vector = recon_prev[3] + down_little;
  if (new_vector <= max && new_vector >= min)
    recon[3] = recon_prev[3] + down_little;
  else
    recon[3] = recon_prev[3] + down_big;
  recon_prev[3] = recon[3];
  if (picture.full_pel_backward_vector) recon[3] = recon[3] << 1;
}


void the_past_reference_picture(int pel[][8], int last_pel[][max_resolution][max_resolution], int recon_right_forward, int recon_down_forward, int block_num){
  int i, j;
  int mb_row = (macroblock.macroblock_address / sequence.mb_width) * 16;
  int mb_col = (macroblock.macroblock_address % sequence.mb_width) * 16;
  int right_for, down_for, right_half_for, down_half_for;
  if (block_num < 4){	// video Page.47 (for luminance)
    right_for = recon_right_forward >> 1;
    down_for = recon_down_forward >> 1;
    right_half_for = recon_right_forward - 2 * right_for;
    down_half_for = recon_down_forward - 2 * down_for;

    mb_row += 8 * (block_num / 2) + down_for;
    mb_col += 8 * (block_num % 2) + right_for;

    if (!right_half_for && !down_half_for){
      for (i = 0; i < 8; i++){
        for (j = 0; j < 8; j++){
          pel[i][j] = last_pel[0][mb_row + i][mb_col + j];
        }
      }
    }
    else if (!right_half_for && down_half_for){
      for (i = 0; i < 8; i++){
        for (j = 0; j < 8; j++){
          pel[i][j] = (last_pel[0][mb_row + i][mb_col + j] + last_pel[0][mb_row + i + 1][mb_col + j]) / 2;
        }
      }
    }
    else if (right_half_for && !down_half_for){
      for (i = 0; i < 8; i++){
        for (j = 0; j < 8; j++){
          pel[i][j] = (last_pel[0][mb_row + i][mb_col + j] + last_pel[0][mb_row + i][mb_col + j + 1]) / 2;
        }
      }
    }
    else if (right_half_for && down_half_for){
      for (i = 0; i < 8; i++){
        for (j = 0; j < 8; j++){
          pel[i][j] = (last_pel[0][mb_row + i][mb_col + j] + last_pel[0][mb_row + i][mb_col + j + 1] + last_pel[0][mb_row + i + 1][mb_col + j] + last_pel[0][mb_row + i + 1][mb_col + j + 1]) / 4;
        }
      }
    }

  }
  else{	// video Page.47 (for chrominance)
    right_for = (recon_right_forward / 2) >> 1;
    down_for = (recon_down_forward / 2) >> 1;
    right_half_for = recon_right_forward / 2 - 2 * right_for;
    down_half_for = recon_down_forward / 2 - 2 * down_for;

    mb_row += 2 * down_for;
    mb_col += 2 * right_for;

    if (!right_half_for && !down_half_for){
      for (i = 0; i < 16; i += 2){
        for (j = 0; j < 16; j += 2){
          pel[i / 2][j / 2] = last_pel[block_num - 3][mb_row + (i)][mb_col + (j)];
        }
      }
    }
    if (!right_half_for && down_half_for){
      for (i = 0; i < 16; i += 2){
        for (j = 0; j < 16; j += 2){
          pel[i / 2][j / 2] = (last_pel[block_num - 3][mb_row + (i)][mb_col + (j)] + last_pel[block_num - 3][mb_row + (i + 2)][mb_col + (j)]) / 2;
        }
      }
    }
    if (right_half_for && !down_half_for){
      for (i = 0; i < 16; i += 2){
        for (j = 0; j < 16; j += 2){
          pel[i / 2][j / 2] = (last_pel[block_num - 3][mb_row + (i)][mb_col + (j)] + last_pel[block_num - 3][mb_row + (i)][mb_col + (j + 2)]) / 2;
        }
      }
    }
    if (right_half_for && down_half_for){
      for (i = 0; i < 16; i += 2){
        for (j = 0; j < 16; j += 2){
          pel[i / 2][j / 2] = (last_pel[block_num - 3][mb_row + (i)][mb_col + (j)] + last_pel[block_num - 3][mb_row + (i)][mb_col + (j + 2)] + 
            last_pel[block_num - 3][mb_row + (i + 2)][mb_col + (j)] + last_pel[block_num - 3][mb_row + (i + 2)][mb_col + (j + 2)]) / 4;
        }
      }
    }
  }
}



void play(){

  double interval = 1000.0 / sequence.fps;
  printf("%lf %lf %d\n", interval, sequence.fps, pic_index);
  namedWindow("itct", CV_WINDOW_AUTOSIZE);
  for (int i = 1; i < pic_index; i++){
    imshow("itct", pic_list[i]);
    waitKey(interval);
    //waitKey(0);
  }
  destroyAllWindows();
}

void convertRGB(int M[][max_resolution][max_resolution]){
  int Y = sequence.vertical_size;
  int X = sequence.horizontal_size;
  Mat img = Mat::zeros(Y, X, CV_8UC3);

  for (int j = 0; j < Y; j++){
    for (int k = 0; k < X; k++){
      img.data[img.step*j + img.channels()*k] = bound(M[0][j][k] + 1.772*(M[1][j][k] - 128));
      img.data[img.step*j + img.channels()*k + 1] = bound(M[0][j][k] - 0.34414*(M[1][j][k] - 128) - 0.71414*(M[2][j][k] - 128));
      img.data[img.step*j + img.channels()*k + 2] = bound(M[0][j][k] + 1.402*(M[2][j][k] - 128));
    }
  }

  pic_list[pic_index] = img;
  pic_index++;
}

void save_to_current_image(int block_num){
  int i, j;
  int block_col, block_row;
  block_col = (macroblock.macroblock_address % sequence.mb_width) * 16;
  block_row = (macroblock.macroblock_address / sequence.mb_width) * 16;

  if (block_num < 4){
    for (i = 0; i < 8; i++){
      for (j = 0; j < 8; j++){
        group.current_image_ycbcr[0][block_row + (block_num / 2) * 8 + i][block_col + (block_num % 2) * 8 + j] = dct_recon[i][j];
      }
    }
  }
  else{
    int n_pic;
    if (block_num == 4)
      n_pic = 1;
    else if (block_num == 5)
      n_pic = 2;

    for (i = 0; i < 8; i++){
      for (j = 0; j < 8; j++){
        group.current_image_ycbcr[n_pic][block_row + 2 * i][block_col + 2 * j] = dct_recon[i][j];
        group.current_image_ycbcr[n_pic][block_row + 2 * i + 1][block_col + 2 * j] = dct_recon[i][j];
        group.current_image_ycbcr[n_pic][block_row + 2 * i][block_col + 2 * j + 1] = dct_recon[i][j];
        group.current_image_ycbcr[n_pic][block_row + 2 * i + 1][block_col + 2 * j + 1] = dct_recon[i][j];
      }
    }
  }
}

void block_header(int n){
  if (pattern_code[n]) {
    //printf("\n=========================[Block %d]=============================\n", n);

    int index = 0;

    for (int i = 0; i < 64; i++)
      dct_zz[i] = 0;

    if (macroblock.macroblock_intra) {
      if (n<4) {

        unsigned char size;
        for (int i = 2; i < 8; i++){
          if (TableHandler::checkDcSize(0, i, lookNBit(i), size)){
            skipNBit(i);
            break;
          }
        }
        block.dct_dc_size_luminance = size;

        if (block.dct_dc_size_luminance != 0){
          block.dct_dc_differential = getNBit(block.dct_dc_size_luminance);
          if (block.dct_dc_differential & (1 << (block.dct_dc_size_luminance - 1)))
            dct_zz[0] = block.dct_dc_differential;
          else
            dct_zz[0] = (-1 << (block.dct_dc_size_luminance)) | (block.dct_dc_differential + 1);
          
          //printf("[Block]: dct_dc_size_luminance %d\n", block.dct_dc_size_luminance);
         // printf("[Block]: dct_dc_differential_luminance %d\n", block.dct_dc_differential);

        }
      }
      else {

        unsigned char size;
        for (int i = 2; i < 8; i++){
          if (TableHandler::checkDcSize(1, i, lookNBit(i), size)){
            skipNBit(i);
            break;
          }
        }
        block.dct_dc_size_chrominance = size;

        if (block.dct_dc_size_chrominance != 0){
          block.dct_dc_differential = getNBit(block.dct_dc_size_chrominance);
          if (block.dct_dc_differential & (1 << (block.dct_dc_size_chrominance - 1)))
            dct_zz[0] = block.dct_dc_differential;
          else
            dct_zz[0] = (-1 << (block.dct_dc_size_chrominance)) | (block.dct_dc_differential + 1);

          //printf("[Block]: dct_dc_size_chrominance %d\n", block.dct_dc_size_chrominance);
          //printf("[Block]: dct_dc_differential_chrominance %d\n", block.dct_dc_differential);
        }
      }
    }
    else {
      int run, level;
      if (lookNBit(1) == 1U){
        skipNBit(1);
        run = 0;
        level = 1;
      }
      else{
        for (int i = 3; i < 17; i++){
          if (TableHandler::checkDctCoefficientTable(i, lookNBit(i), run, level)){
            skipNBit(i);
            break;
          }
        }
      }

      if (run == -1 && level == -1){
        run = getNBit(6);
        if (TableHandler::checkDctCoefficientEscapeTable(8, lookNBit(8), level))
          skipNBit(8);
        else if (TableHandler::checkDctCoefficientEscapeTable(16, lookNBit(16), level))
          skipNBit(16);
      }
      else{
        if (getNBit(1) == 1U)
          level = -level;
      }

      dct_zz[run] = level;
      index = run;

      //printf("[Block]: dct_coeff_first Run: %d Level: %d\n", run, level);
    }
    
    if (picture.picture_coding_type != 4) {
      while (nextbits(2) != 0x2){
        int run, level;
        for (int i = 2; i < 17; i++){
          if (TableHandler::checkDctCoefficientTable(i, lookNBit(i), run, level)){
            skipNBit(i);
            if (run == -1 && level == -1){
              run = getNBit(6);
              if (TableHandler::checkDctCoefficientEscapeTable(8, lookNBit(8), level))
                skipNBit(8);
              else if (TableHandler::checkDctCoefficientEscapeTable(16, lookNBit(16), level))
                skipNBit(16);

              index += run + 1;
              dct_zz[index] = level;
            }
            else{
              index += run + 1;
              if (getNBit(1) == 1U)
                level = -level;
              dct_zz[index] = level;
            }

            //printf("[Block]: dct_coeff_next Run: %d Level: %d\n", run, level);
            
            break;
          }
        }
      }
      skipNBit(2);
    }
  }
}

void intra(){
  for (int m = 0; m<8; m++) {
    for (int n = 0; n<8; n++) {
      int i1 = m * 8 + n;
      int i2 = TableHandler::zigzagOrder[i1];
      dct_recon[m][n] = (2 * dct_zz[i2] * slice.quantizer_scale * sequence.intra_quantizer_matrix[i1]) / 16;
      if ((dct_recon[m][n] & 1) == 0)
        dct_recon[m][n] = dct_recon[m][n] - sign(dct_recon[m][n]);
      if (dct_recon[m][n] > 2047) 
        dct_recon[m][n] = 2047;
      if (dct_recon[m][n] < -2048) 
        dct_recon[m][n] = -2048;
    }
  }
}

void non_intra(){
  for (int m = 0; m<8; m++) {
    for (int n = 0; n<8; n++) {
      int i1 = m * 8 + n;
      int i2 = TableHandler::zigzagOrder[i1];
      dct_recon[m][n] = (((2 * dct_zz[i2]) + sign(dct_zz[i2])) * slice.quantizer_scale * sequence.non_intra_quantizer_matrix[i1]) / 16;
      if ((dct_recon[m][n] & 1) == 0)
        dct_recon[m][n] = dct_recon[m][n] - sign(dct_recon[m][n]);
      if (dct_recon[m][n] > 2047) 
        dct_recon[m][n] = 2047;
      if (dct_recon[m][n] < -2048) 
        dct_recon[m][n] = -2048;
      if (dct_zz[i2] == 0)
        dct_recon[m][n] = 0;
    }
  }
}

void intra_first_y(){
  intra();
  dct_recon[0][0] = dct_zz[0] * 8;
  if ((macroblock.macroblock_address - slice.past_intra_address > 1))
    dct_recon[0][0] = 128 * 8 + dct_recon[0][0];
  else
    dct_recon[0][0] = slice.dct_dc_y_past + dct_recon[0][0];
  
  slice.dct_dc_y_past = dct_recon[0][0];
}

void intra_remain_y(){
  intra();
  dct_recon[0][0] = slice.dct_dc_y_past + dct_zz[0] * 8;
  slice.dct_dc_y_past = dct_recon[0][0];
}

void intra_cb(){
  intra();
  dct_recon[0][0] = dct_zz[0] * 8;
  if ((macroblock.macroblock_address - slice.past_intra_address > 1))
    dct_recon[0][0] = 128 * 8 + dct_recon[0][0];
  else
    dct_recon[0][0] = slice.dct_dc_cb_past + dct_recon[0][0];
  slice.dct_dc_cb_past = dct_recon[0][0];
}

void intra_cr(){
  intra();
  dct_recon[0][0] = dct_zz[0] * 8;
  if ((macroblock.macroblock_address - slice.past_intra_address > 1))
    dct_recon[0][0] = 128 * 8 + dct_recon[0][0];
  else
    dct_recon[0][0] = slice.dct_dc_cr_past + dct_recon[0][0];
  slice.dct_dc_cr_past = dct_recon[0][0];
}

void ycbcr_past_reset(){
  slice.dct_dc_y_past = 1024;
  slice.dct_dc_cb_past = 1024;
  slice.dct_dc_cr_past = 1024;
}

void motion_compensation(int motion_vector[][8]){
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      dct_recon[i][j] += motion_vector[i][j];
    }
  }
}

void average_matrix(int pel_avg[][8], int pel_first[][8], int pel_second[][8]){
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      pel_avg[i][j] = (pel_first[i][j] + pel_second[i][j]) / 2;
    }
  }
}

void macroblock_header(){
  while (nextbits(11) == 0xf)
    macroblock.macroblock_stuffing = getNBit(11);

  macroblock.macroblock_escape = 0;
  while (nextbits(11) == 0x8){
    skipNBit(11);
    macroblock.macroblock_escape++;
  }

  for (int i = 1; i < 12; i++){
    if (TableHandler::checkMacroblockAddressingTable(i, lookNBit(i), macroblock.macroblock_address_increment)){
      skipNBit(i);
      break;
    }
  }

  macroblock.macroblock_address_increment += 33 * macroblock.macroblock_escape;
  for (int i = 1; i < macroblock.macroblock_address_increment; i++){
    macroblock.macroblock_address = previous_macroblock_address + i;
    if (picture.picture_coding_type == 2){
      for (int j = 0; j < 6; j++){
        the_past_reference_picture(dct_recon, group.forward_image_ycbcr, 0, 0, j);
        save_to_current_image(j);
      }
    }
    else if (picture.picture_coding_type == 3){
      for (int j = 0; j < 6; j++){
        if (macroblock_previous_motion_forward && macroblock_previous_motion_backward){
          the_past_reference_picture(pel_for, group.forward_image_ycbcr, 0, 0, j);
          the_past_reference_picture(pel_back, group.backward_image_ycbcr, 0, 0, j);
          average_matrix(pel_avg, pel_for, pel_back);
        }
        else if (macroblock_previous_motion_forward)
          the_past_reference_picture(pel_avg, group.forward_image_ycbcr, 0, 0, j);
        else if (macroblock_previous_motion_backward)
          the_past_reference_picture(pel_avg, group.backward_image_ycbcr, 0, 0, j);
        memset(dct_recon, 0, sizeof(dct_recon));
        motion_compensation(pel_avg);
        save_to_current_image(j);
      }
    }
  }

  macroblock.macroblock_address = previous_macroblock_address + macroblock.macroblock_address_increment;
  previous_macroblock_address = macroblock.macroblock_address;

  if (macroblock.macroblock_address_increment > 1){
    ycbcr_past_reset();
  }

  if (macroblock.macroblock_address_increment > 1 && picture.picture_coding_type == 2){
    recon_prev[0] = 0;
    recon_prev[1] = 0;
  }

  unsigned char block_type;
  for (int i = 1; i < 7; i++){
    if (TableHandler::checkMacroblockTypeTable(picture.picture_coding_type, i, lookNBit(i), block_type)){
      skipNBit(i);
      break;
    }
  }

  macroblock.macroblock_type = block_type;
  macroblock.macroblock_quant = block_type & 0x10;
  macroblock.macroblock_motion_forward = block_type & 0x08;
  macroblock.macroblock_motion_backward = block_type & 0x04;
  macroblock.macroblock_pattern = block_type & 0x02;
  macroblock.macroblock_intra = block_type & 0x01;

  macroblock_previous_motion_forward = macroblock.macroblock_motion_forward;
  macroblock_previous_motion_backward = macroblock.macroblock_motion_backward;

  if (macroblock.macroblock_quant)
    slice.quantizer_scale = getNBit(5);

  if (macroblock.macroblock_motion_forward) {
    for (int i = 1; i < 12; i++){
      if (TableHandler::checkMotionVectorTable(i, lookNBit(i), macroblock.motion_horizontal_forward_code)){
        skipNBit(i);
        break;
      }
    }
    if ((picture.forward_f != 1) && (macroblock.motion_horizontal_forward_code != 0))
      macroblock.motion_horizontal_forward_r = getNBit(picture.forward_r_size);
    for (int i = 1; i < 12; i++){
      if (TableHandler::checkMotionVectorTable(i, lookNBit(i), macroblock.motion_vertical_forward_code)){
        skipNBit(i);
        break;
      }
    }
    if ((picture.forward_f != 1) && (macroblock.motion_vertical_forward_code != 0))
      macroblock.motion_vertical_forward_r = getNBit(picture.forward_r_size);

    forward_motion_prediction();
  }
  else if(picture.picture_coding_type == 2){
    recon[0] = 0;
    recon[1] = 0;
    recon_prev[0] = 0;
    recon_prev[1] = 0;
  }

  if (macroblock.macroblock_motion_backward) {

    for (int i = 1; i < 12; i++){
      if (TableHandler::checkMotionVectorTable(i, lookNBit(i), macroblock.motion_horizontal_backward_code)){
        skipNBit(i);
        break;
      }
    }

    if ((picture.backward_f != 1) && (macroblock.motion_horizontal_backward_code != 0))
      macroblock.motion_vertical_backward_code = getNBit(picture.backward_r_size);

    for (int i = 1; i < 12; i++){
      if (TableHandler::checkMotionVectorTable(i, lookNBit(i), macroblock.motion_vertical_backward_code)){
        skipNBit(i);
        break;
      }
    }

    if ((picture.backward_f != 1) && (macroblock.motion_vertical_backward_code != 0))
      macroblock.motion_vertical_backward_r = getNBit(picture.backward_r_size);

    backward_motion_prediction();
  }

  unsigned char pattern = 0;
  if (macroblock.macroblock_pattern){
    for (int i = 3; i < 10; i++){
      if (TableHandler::checkCodedBlockPatternTable(i, lookNBit(i), pattern)){
        skipNBit(i);
        break;
      }
    }
  }

  //printf("\n=========================[Macroblock Header %d haha:%d]=============================\n", macroblock.macroblock_address, haha); 
  //printf("[Macroblock Header]: macroblock_address_increment %d\n", macroblock.macroblock_address_increment);
  //printf("[Macroblock Header]: macroblock_type %d\n", macroblock.macroblock_type);
  //printf("[Macroblock Header]: macroblock_quant %d\n", macroblock.macroblock_quant);
  //printf("[Macroblock Header]: macroblock_motion_forward %d\n", macroblock.macroblock_motion_forward);
  //printf("[Macroblock Header]: macroblock_motion_backward %d\n", macroblock.macroblock_motion_backward);
  //printf("[Macroblock Header]: macroblock_pattern %d\n", macroblock.macroblock_pattern);
  //printf("[Macroblock Header]: macroblock_intra %d\n", macroblock.macroblock_intra);
  //printf("[Macroblock Header]:ForMV(%d, %d)\n", recon[0], recon[1]);
  //if (haha > 1)system("pause");

  if (picture.picture_coding_type == 3 && macroblock.macroblock_intra){
    recon_prev[0] = 0;
    recon_prev[1] = 0; 
    recon_prev[2] = 0;
    recon_prev[3] = 0;
  }

  for (int i = 0; i < 6; i++){
    pattern_code[i] = 0;
    if (macroblock.macroblock_intra || (pattern & (1 << (5 - i))))
      pattern_code[i] = 1;
    block_header(i);

    if (macroblock.macroblock_intra){
      if (i == 0) intra_first_y();
      else if (i > 0 && i < 4)	intra_remain_y();
      else if (i == 4)	intra_cb();
      else if (i == 5)	intra_cr();

      idct((int*)dct_recon);
      save_to_current_image(i);
    }
    else{
      ycbcr_past_reset();
      
      if (pattern_code[i] == 1){
        if (picture.picture_coding_type == 2){
          the_past_reference_picture(pel_for, group.forward_image_ycbcr, recon[0], recon[1], i);
          non_intra();
          idct((int *)dct_recon);
          motion_compensation(pel_for);
          save_to_current_image(i);
        }
        else if (picture.picture_coding_type == 3){
          if (macroblock.macroblock_motion_forward && macroblock.macroblock_motion_backward){
            the_past_reference_picture(pel_for, group.forward_image_ycbcr, recon[0], recon[1], i);
            the_past_reference_picture(pel_back, group.backward_image_ycbcr, recon[2], recon[3], i);
            average_matrix(pel_avg, pel_for, pel_back);
          }
          else if (macroblock.macroblock_motion_forward)
            the_past_reference_picture(pel_avg, group.forward_image_ycbcr, recon[0], recon[1], i);
          else if (macroblock.macroblock_motion_backward)
            the_past_reference_picture(pel_avg, group.backward_image_ycbcr, recon[2], recon[3], i);

          non_intra();
          idct((int *)dct_recon);
          motion_compensation(pel_avg);
          save_to_current_image(i);
        }
      }
      else{
        if (picture.picture_coding_type == 2){
          the_past_reference_picture(dct_recon, group.forward_image_ycbcr, recon[0], recon[1], i);
          save_to_current_image(i);
        }
        else if (picture.picture_coding_type == 3){
          if (macroblock.macroblock_motion_forward && macroblock.macroblock_motion_backward)
          {
            the_past_reference_picture(pel_for, group.forward_image_ycbcr, recon[0], recon[1], i);
            the_past_reference_picture(pel_back, group.backward_image_ycbcr, recon[2], recon[3], i);
            average_matrix(pel_avg, pel_for, pel_back);
          }
          else if (macroblock.macroblock_motion_forward)
            the_past_reference_picture(pel_avg, group.forward_image_ycbcr, recon[0], recon[1], i);
          else if (macroblock.macroblock_motion_backward)
            the_past_reference_picture(pel_avg, group.backward_image_ycbcr, recon[2], recon[3], i);

          memset(dct_recon, 0, sizeof(dct_recon));
          motion_compensation(pel_avg);
          save_to_current_image(i);
        }
      }

    }

  }
  if (picture.picture_coding_type == 4)
    skipNBit(1);

  if (macroblock.macroblock_intra == 1)
    slice.past_intra_address = macroblock.macroblock_address;
}

bool in_slice_start_code_range(unsigned int code){
  if (code >= slice_start_codes_min && code <= slice_start_codes_min)
    return true;
  else
    return false;
}

void slice_header(){

  unsigned int start_code = getNBit(32);
  if (!in_slice_start_code_range(start_code))
    printf("[Slice Header]: Wrong slice start code\n");

  unsigned int slice_vertical_position = start_code & 0xff;

  previous_macroblock_address = (slice_vertical_position - 1)*sequence.mb_width - 1;

  slice.quantizer_scale = getNBit(5);
  while (nextbits(1) == 1) {
    slice.extra_bit_slice = getNBit(1);
    slice.extra_information_slice = getNBit(8);
  }
  slice.extra_bit_slice = getNBit(1);

  slice.past_intra_address = -2;
  ycbcr_past_reset();

  /*printf("\n=========================[Slice Header]=============================\n");
  printf("[Slice Header]: quantizer_scale %d\n", slice.quantizer_scale);
  printf("[Slice Header]: extra_bit_slice %d\n", slice.extra_bit_slice);*/

  
  recon[0] = 0;
  recon[1] = 0;
  recon[2] = 0;
  recon[3] = 0;
  recon_prev[0] = 0;
  recon_prev[1] = 0; 
  recon_prev[2] = 0;
  recon_prev[3] = 0;

  do {
    macroblock_header();
  } while (nextbits(23) != 0);
  next_start_code();
}


void picture_header() {
  if (getNBit(32) != picture_start_code)
    printf("[Picture Header]: Wrong picture start code\n");

  picture.temporal_reference = getNBit(10);
  picture.picture_coding_type = getNBit(3);
  picture.vbv_delay = getNBit(16);

  if (picture.picture_coding_type == 2 || picture.picture_coding_type == 3) {
    picture.full_pel_forward_vector = getNBit(1);
    picture.forward_f_code = getNBit(3);
    picture.forward_r_size = picture.forward_f_code - 1;
    picture.forward_f = 1 << picture.forward_r_size;
  }

  if (picture.picture_coding_type == 3) {
    picture.full_pel_backward_vector = getNBit(1);
    picture.backward_f_code = getNBit(3);
    picture.backward_r_size = picture.backward_f_code - 1;
    picture.backward_f = 1 << picture.backward_r_size;
  }

  while (nextbits(1) == 1U) {
    picture.extra_bit_picture = getNBit(1);
    picture.extra_information_picture = getNBit(8);
  }
  picture.extra_bit_picture = getNBit(1);

  next_start_code();

  if (nextbits(32) == extension_start_code) {
    skipNBit(32);
    while (nextbits(24) != start_code_begin) {
      picture.picture_extension_data = getNBit(8);
    }
    next_start_code();
  }

  if (nextbits(32) == user_data_start_code) {
    skipNBit(32);
    while (nextbits(24) != start_code_begin) {
      picture.user_data = getNBit(8);
    }
    next_start_code();
  }

  printf("\n=========================[Picture Header %d]=============================\n", haha); haha++;// qq = 0;
  /*printf("[Picture Header]: temporal_reference %d\n", picture.temporal_reference);
  printf("[Picture Header]: picture_coding_type %d\n", picture.picture_coding_type);
  printf("[Picture Header]: vbv_delay %d\n", picture.vbv_delay);
  printf("[Picture Header]: extra_bit_picture %d\n", picture.extra_bit_picture);*/

  if (picture.picture_coding_type == 1 || picture.picture_coding_type == 2){
    memcpy(group.forward_image_ycbcr, group.backward_image_ycbcr, sizeof(group.current_image_ycbcr));
    convertRGB(group.forward_image_ycbcr);
  }

  do {
    slice_header();
  } while (in_slice_start_code_range(nextbits(32)));

  if (picture.picture_coding_type == 1 || picture.picture_coding_type == 2){
    memcpy(group.forward_image_ycbcr, group.backward_image_ycbcr, sizeof(group.current_image_ycbcr));
    memcpy(group.backward_image_ycbcr, group.current_image_ycbcr, sizeof(group.current_image_ycbcr));
  }
  else{	// B
    convertRGB(group.current_image_ycbcr);
  }

}

void group_header(){
  if (getNBit(32) != group_start_code)
    printf("[Group Of Pictures]: Wrong group start code\n");

  group.time_code = getNBit(25);
  group.closed_gop = getNBit(1);
  group.broken_link = getNBit(1);

  next_start_code();

  if (nextbits(32) == extension_start_code) {
    skipNBit(32);
    while (nextbits(24) != start_code_begin) {
      group.group_extension_data = getNBit(8);
    }
    next_start_code();
  }

  if (nextbits(32) == user_data_start_code) {
    skipNBit(32);
    while (nextbits(24) != start_code_begin) {
      group.user_data = getNBit(8);
    }
    next_start_code();
  }

  do {
    picture_header();
  } while (nextbits(32) == picture_start_code);
}

void sequence_header() {
  if (getNBit(32) != sequence_header_code)
    printf("[Sequence Header]: Wrong sequence header code\n");

  sequence.horizontal_size = getNBit(12);
  sequence.mb_width = (sequence.horizontal_size + 15) / 16;

  sequence.vertical_size = getNBit(12);
  sequence.mb_height = (sequence.vertical_size + 15) / 16;

  sequence.pel_aspect_ratio = getNBit(4);
  sequence.picture_rate = getNBit(4);
  TableHandler::checkPictureRateTable(sequence.picture_rate, sequence.fps);
  sequence.bit_rate = getNBit(18);
  sequence.marker_bit = getNBit(1);
  sequence.vbv_buffer_size = getNBit(10);
  sequence.constrained_parameter_flag = getNBit(1);

  sequence.load_intra_quantizer_matrix = getNBit(1);
  if (sequence.load_intra_quantizer_matrix){
    for (int i = 0; i < 64; i++){
      sequence.intra_quantizer_matrix[i] = getNBit(8);
    }
  }
  else{
    for (int i = 0; i < 64; i++){
      sequence.intra_quantizer_matrix[i] = TableHandler::defaultIntraQuantizerMatrix[i];
    }
  }

  sequence.load_non_intra_quantizer_matrix = getNBit(1);
  if (sequence.load_non_intra_quantizer_matrix){
    for (int i = 0; i < 64; i++){
      sequence.non_intra_quantizer_matrix[i] = getNBit(8);
    }
  }
  else{
    for (int i = 0; i < 64; i++){
      sequence.non_intra_quantizer_matrix[i] = TableHandler::defaultNonIntraQuantizerMatrix[i];
    }
  }

  next_start_code();

  if (nextbits(32) == extension_start_code) {
    skipNBit(32);
    while (nextbits(24) != start_code_begin) {
      sequence.sequence_extension_data = getNBit(8);
    }
    next_start_code();
  }
  if (nextbits(32) == user_data_start_code) {
    skipNBit(32);
    while (nextbits(24) != start_code_begin) {
      sequence.user_data = getNBit(8);
    }
    next_start_code();
  }
}
//int kkk = 0;
void video_sequence() {
  next_start_code();
  do {
    sequence_header();
    do {
      group_header();
      //cout << "group" << kkk++ << endl; system("pause");
    } while (lookNBit(32) == group_start_code);
  } while (lookNBit(32) == sequence_header_code);
  if (getNBit(32) != sequence_end_code)
    printf("[Video Sequence]: Wrong sequence end code\n");
}