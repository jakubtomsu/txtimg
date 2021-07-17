
#include <stdio.h>
#include <math.h>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_STRINGS_USERMSG
#include "stb_image.h"

#define TXTIMG_DEBUG 1

#if TXTIMG_DEBUG
#define PRINTBOOL(b) printf(#b " = %s\n", (b) ? "true" : "false");
#define PRINTINT(i) printf(#i " = %i\n", (i));
#define TXTIMG_ERROR(condition, ...) if(condition){ printf("error (" #condition ") " __VA_ARGS__); printf("\n"); exit(-1);}
#else
#define PRINTBOOL(b)
#define PRINTINT(i)
#define TXTIMG_ERROR(condition, ...) if(condition){ printf("error: " __VA_ARGS__); printf("\n"); exit(-1); }
#endif
char pxvalchars[] = " .,'-~+:;^<=/i![?038$#@";

static char eval(int val){

	const int step = 255 / sizeof(pxvalchars);
	int idx = val / step;
	if(idx >= sizeof(pxvalchars)-1) idx = sizeof(pxvalchars) - 2;

	return pxvalchars[idx];
	return ('#');
}

static unsigned int pxidx(int x, int y, int imgx, int imgy, int imgch){
	unsigned int idx = x + (y * imgx);
	return idx * imgch;
}

static bool isdigit(char c){
	 return (c >= '0' && c <= '9');
}

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


int main(int argc, const char* argv[]){
	
	TXTIMG_ERROR(argc <= 1, "no input file");

	if(argc == 2 && strcmp(argv[1], "--help")<=0){
		printf("\tfirst argument      file to open\n"
			"\tsecond argument     pixel step (how many pixels should a single character contain)\n"
			"\t--help              display this message\n");
		return 0;
	}


	int step = 6;
	if(argc >= 3) step = uint_parse(argv[2]);
	const int steph = step / 2;
	const int steparea = (step - 1) *  (steph - 1);

	printf("loading image ... ");
	int imgx,imgy,imgch;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imgdata = stbi_load(argv[1], &imgx, &imgy, &imgch, 0);
	if(!imgdata) printf("image loading error: %s\n", stbi_failure_reason());
	else printf("done\n");
	
	PRINTINT(imgx);
	PRINTINT(imgy);
	PRINTINT(imgch);

	const unsigned int pxcount = imgx * imgy;
	const unsigned int bcount = pxcount * imgch;
	PRINTINT(pxcount);
	PRINTINT(bcount);


	for(int y = imgy-step; y > 0; y-= step){
		for(int x = 0; x < imgx; x+= steph){

			int val = 0;
			
			for(int aax = x; aax < x + steph - 1; aax ++){
				for(int aay = y; aay < y + step - 1; aay ++){
					unsigned int idx = pxidx(aax,aay,imgx,imgy,imgch);
					
					TXTIMG_ERROR(idx < 0,"idx = %i, x = %i, y = %i", idx, aax, aay);
					TXTIMG_ERROR(idx >= bcount, "idx = %i, x = %i, y = %i", idx, aax, aay);
					val += imgdata[idx] + imgdata[idx + 1] +imgdata[idx + 2];
                       		}
			}
			
			val /= steparea * 3;
			printf("%c", eval(val));
		}
		printf("\n");
	}


	return 0;
}
