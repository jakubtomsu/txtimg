
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_STRINGS_USERMSG
#include "stb_image.h"


#ifdef TXTIMG_DEBUG
#define PRINTBOOL(b) printf(#b " = %s\n", (b) ? "true" : "false");
#define PRINTINT(i) printf(#i " = %i\n", (i));
#define TXTIMG_ERROR(condition, ...) if(condition){ printf("error (" #condition ") " __VA_ARGS__); printf("\n"); exit(-1);}
#else
#define PRINTBOOL(b)
#define PRINTINT(i)
#define TXTIMG_ERROR(condition, ...) if(condition){ printf("error: " __VA_ARGS__); printf("\n"); exit(-1); }
#endif

// character gradient
char pxvalchars[] = " .,'-~+:;^<=/i![?038$#@";

// pick a character for value from 0.0 to 1.0
static char eval(float val){
	if(val >= 0.99f) val = 0.99f;
	else if(val <= 0.01f) val = 0.01f;
	int idx = val * (sizeof(pxvalchars) - 1);

	return pxvalchars[idx];
}

// calculate 1D pixel index from 2D position
static unsigned int pxidx(int x, int y, int imgx, int imgy, int imgch){
	unsigned int idx = x + (y * imgx);
	return idx * imgch;
}

// check if character is digit
static bool isdigit(char c){
	 return (c >= '0' && c <= '9');
}

// get number from a string
unsigned int uint_parse(const char* str){
	unsigned int val = 0;
	char* end = ((char*)str) + strlen(str);

	unsigned int count = 0;
	while(end != str){
		end --;
		if(isdigit(*end)){
			val += (((int)*end) - (int)'0') * (int)powf(10, count);
			count++;
		}
	}
	
	return val;
}

float logit_function(float x){
	return log(x / (1.0f - x));
}

float lerpf(float s, float e, float t) {
	return (s * (1.0f - t)) + (e * t);
}

int main(int argc, const char* argv[]){
	
	TXTIMG_ERROR(argc <= 1, "no input file");

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


	int step = 6;
	if(argc >= 3) step = uint_parse(argv[2]);
	const int steph = step / 2;
	const int steparea = (step - 1) *  (steph - 1);
	float sharpen = 0;
	if(argc >= 4) sharpen = (float)uint_parse(argv[3]) / 100.0f;
	float multiplier = 1.0f;
	if(argc >= 5) multiplier = (float)uint_parse(argv[4]) / 100.0f;


	printf("loading image ... ");
	int imgx,imgy,imgch;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imgdata = stbi_load(argv[1], &imgx, &imgy, &imgch, 0);
	if(!imgdata) printf("image loading error: %s\n", stbi_failure_reason());
	else printf("done\n");

	const unsigned int pxcount = imgx * imgy;
	const unsigned int bcount = pxcount * imgch;
	
#ifdef TXTIMG_DEBUG
	PRINTINT(imgx);
	PRINTINT(imgy);
	PRINTINT(imgch);
	PRINTINT(pxcount);
	PRINTINT(bcount);
#endif

	for(int y = imgy-step; y > 0; y-= step){
		for(int x = 0; x < imgx; x+= steph){

			float val = 0;
			
			for(int aax = x; aax < x + steph - 1; aax ++){
				for(int aay = y; aay < y + step - 1; aay ++){
					unsigned int idx = pxidx(aax,aay,imgx,imgy,imgch);
					
					TXTIMG_ERROR(idx < 0,"idx = %i, x = %i, y = %i", idx, aax, aay);
					TXTIMG_ERROR(idx >= bcount, "idx = %i, x = %i, y = %i", idx, aax, aay);
					val += imgdata[idx] + imgdata[idx + 1] +imgdata[idx + 2];
                       		}
			}
			
			val /= (float)(steparea * 3 * 255);
			float logit = logit_function(val);
			val = lerpf(val, logit, sharpen);
			val *= multiplier;

			printf("%c", eval(val));
		}
		printf("\n");
	}
	printf("time = %f s", (float)clock() / (float)CLOCKS_PER_SEC);


	return 0;
}
