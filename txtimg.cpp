
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_STRINGS_USERMSG
#include "stb_image.h"


#ifdef TXTIMG_DEBUG
#define TXIMG_PRINTBOOL(b) printf(#b " = %s\n", (b) ? "true" : "false");
#define TXIMG_PRINTINT(i) printf(#i " = %i\n", (i));
#define TXTIMG_ERROR(condition, ...) if(condition){ printf("error (" #condition ") " __VA_ARGS__); printf("\n"); exit(-1);}
#else
#define TXIMG_PRINTBOOL(b)
#define TXIMG_PRINTINT(i)
#define TXTIMG_ERROR(condition, ...) if(condition){ printf("error: " __VA_ARGS__); printf("\n"); exit(-1); }
#endif

// character gradient
char txtimg_char_gradient[] =
//" .,'-~+:;^<=/i![?038$#@";
" .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

// pick a character for value from 0.0 to 1.0
static char txtimg_evaluate(float val){
	if(val >= 0.99f) val = 0.99f;
	else if(val <= 0.01f) val = 0.01f;
	int idx = val * (sizeof(txtimg_char_gradient) - 1);

	return txtimg_char_gradient[idx];
}

// calculate 1D pixel index from 2D position
static unsigned int txtimg_px_idx(int x, int y, int imgx, int imgy, int imgch){
	unsigned int idx = x + (y * imgx);
	return idx * imgch;
}

// check if character is digit
static bool txtimg_char_is_digit(char c){
	 return (c >= '0' && c <= '9');
}

// get uint from a string
unsigned int txtimg_uint_parse(const char* str){
	unsigned int val = 0;
	char* end = ((char*)str) + strlen(str);

	unsigned int count = 0;
	while(end != str){
		end --;
		if(txtimg_char_is_digit(*end)){
			val += (((int)*end) - (int)'0') * (int)powf(10, count);
			count++;
		}
	}
	
	return val;
}

// sharpness function. inverse of sigmoid function, pushes x towards 0 or 1 (which is closer)
float txtimg_logit_function(float x){
	return log(x / (1.0f - x));
}

float txtimg_lerpf(float s, float e, float t) {
	return (s * (1.0f - t)) + (e * t);
}

int main(int argc, const char* argv[]){
	
	TXTIMG_ERROR(argc <= 1, "no input file");

	if (argc == 1) {
		printf("hello! if you're wondering how to use this thing, use --help flag when running this program again!\n");
		return 0;
	}

	if(argc == 2 && strcmp(argv[1], "--help")<=0){
		printf(
			"\tfirst argument      file to open\n"
			"\tsecond argument     pixel step (how many pixels should a single character contain)\n"
			"\tthird argument      sharpen (0 - 100)\n"
			"\tfourth argument     multiplier (fractions of 100)"
			"\t--help              display this message\n"
			);
		return 0;
	}

	// "parse" arguments
	int step = 6;
	if(argc >= 3) step = txtimg_uint_parse(argv[2]);
	const int steph = step / 2;
	const int steparea = (step - 1) *  (steph - 1);
	float sharpen = 0;
	if(argc >= 4) sharpen = (float)txtimg_uint_parse(argv[3]) / 100.0f;
	float multiplier = 1.0f;
	if(argc >= 5) multiplier = (float)txtimg_uint_parse(argv[4]) / 100.0f;


	// load image
	printf("loading image ... ");
	int imgx,imgy,imgch;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imgdata = stbi_load(argv[1], &imgx, &imgy, &imgch, 0);
	if (!imgdata) {
		printf("failed\n");
		TXTIMG_ERROR(!imgdata, "when loading image: %s\n", stbi_failure_reason());
	}
	else printf("ok\n");


	const unsigned int pxcount = imgx * imgy;
	const unsigned int bcount = pxcount * imgch;
	
#ifdef TXTIMG_DEBUG
	TXIMG_PRINTINT(imgx);
	TXIMG_PRINTINT(imgy);
	TXIMG_PRINTINT(imgch);
	TXIMG_PRINTINT(pxcount);
	TXIMG_PRINTINT(bcount);
#endif

	// print image to console
	for(int y = imgy-step; y > 0; y-= step) {
		for(int x = 0; x < imgx; x+= steph) {
			float val = 0;
			
			// "anti aliasing"
			// take average value of all pixels that we skipped
			for(int aax = x; aax < x + steph - 1; aax ++){
				for(int aay = y; aay < y + step - 1; aay ++){
					unsigned int idx = txtimg_px_idx(aax,aay,imgx,imgy,imgch);
					
					TXTIMG_ERROR(idx < 0,"idx = %i, x = %i, y = %i", idx, aax, aay);
					TXTIMG_ERROR(idx >= bcount, "idx = %i, x = %i, y = %i", idx, aax, aay);
					val += imgdata[idx] + imgdata[idx + 1] +imgdata[idx + 2];
                       		}
			}
			
			// modify value by sharpness etc
			val /= (float)(steparea * 3 * 255);
			float logit = txtimg_logit_function(val);
			val = txtimg_lerpf(val, logit, sharpen);
			val *= multiplier;

			printf("%c", txtimg_evaluate(val));
		}
		printf("\n");
	}
	printf("time = %f s", (float)clock() / (float)CLOCKS_PER_SEC);


	return 0;
}
