#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#ifdef _WIN32
#include <sys/stat.h>
#endif
#define boms sizeof(utf8bom) / sizeof(utf8bom[0])
#define bufsiz 16777216
#pragma warning(disable:4996)
const char* h = "\nSTARTING VIOL.......\n=======================================================\n====THIS FIL(L)E HAS BEEN VIOLED SUCCESSFULLY-5IGI0====\n=======================================================\n";
const char* fext = ".violed";
const unsigned char utf8bom[3] = { 0xEF, 0xBB , 0xBF };
int enc(char* filnm, char* outnm) {
	unsigned char hsiz = sizeof(unsigned char) + sizeof(int32_t) + strlen(h);
	size_t br = 0;
	FILE* vv = fopen(filnm, "rb");
	if (vv == NULL) { puts("Error: could not open file"); return 1; }
  int strsiz=strlen(filnm);
  if(outnm==NULL) {
    outnm=malloc(strsiz+strlen(fext)+1);
    memcpy(outnm,filnm,strsiz);
    memcpy(outnm+strsiz,fext,strlen(fext));
    outnm[strsiz+strlen(fext)]='\0';
  }
  FILE* out = fopen(outnm, "wb");
	if (out == NULL) { puts("Error: could not open file"); return 1; }
	fwrite(utf8bom, 1, boms, out);
	fwrite((const void*)&hsiz, sizeof(unsigned char), 1, out);
	int32_t ut = time(NULL);
	uint8_t utimeb[4] = { (ut >> 24) & 0xFF , (ut >> 16) & 0xFF , (ut >> 8) & 0xFF , ut & 0xFF };
	fwrite(utimeb, sizeof(int32_t), 1, out);
	fputs(h, out);
	fputs(filnm, out);
	fputc('\0', out);
  int icnt=0;
  unsigned char* buf = malloc(bufsiz>>3);
  unsigned char* res = malloc(bufsiz);
  if(buf==NULL||res==NULL){
    puts("Error: failed to allocate memory for the 5IGIfier. Reduce buffer size and recompile.");
    goto clean;
  }
	while ((br = fread(buf, 1, bufsiz>>3, vv)) > 0) {
		for (size_t i = 0; i < br; i++) {
			register unsigned char b = buf[i];
			for (signed char j = 7; j >= 0; j--) {
				if ((b & (1 << j)))res[(i << 3) - j + 7] = '\'';
				else res[(i << 3) - j + 7] = '-';
			}
		}
    fwrite(res, 1, br<<3, out);
	}
  free(buf);
  free(res);
  buf=NULL;
  res=NULL;
  clean:;
	fclose(vv);
	fclose(out);
  if(outnm==NULL){free(outnm);outnm=NULL;}
	return 0;
}
int32_t dec(char opt, char* filnm, char* outnm) {
  unsigned char hsiz;
  int32_t ut;
  uint8_t utimeb[4];
  char* encfilnm;
  int strsiz;
	size_t cr = 0;
	FILE* unvv = fopen(filnm, "rb");
	if (!unvv) { puts("Error: could not open file"); return 1; }
	fseek(unvv, boms, SEEK_SET);
  if(opt!='r'){
    hsiz = fgetc(unvv);
    fread(utimeb, sizeof(uint8_t), boms, unvv);
    ut = (utimeb[0] << 24) | (utimeb[1] << 16) | (utimeb[2] << 8) | utimeb[3];
    if (opt == 'u' && outnm == NULL) {
      fclose(unvv);
      return ut;
    }
    strsiz=0;
    fseek(unvv,hsiz+boms,SEEK_SET);
    encfilnm = malloc(512*sizeof(unsigned char));
    while(++strsiz<=512&&(encfilnm[strsiz-1]=fgetc(unvv)));
    if(encfilnm[strsiz-1]){
      fclose(unvv);
      free(encfilnm);
      encfilnm = NULL;
      puts("Error: file corrupted (violed by 5IGI0)");
      return 2;
    }
    if (opt == 'd' && outnm == NULL) {
      outnm = encfilnm;
    }
    if (opt == 'f') {
      puts(encfilnm);
      goto clean;
    }
  }
  int diff = opt=='r'?0:hsiz + boms + strsiz;
#ifdef _WIN32
	struct _stat s;
	_stat(filnm, &s);
#else
	fseek(unvv, 0L, SEEK_END);
#endif
	if ((
#ifdef _WIN32
		s.st_size
#else
		ftell(unvv)
#endif
		- diff) % 8) {
		puts("Error: file corrupted (violed by 5IGI0)"); return 2;
	}
#ifndef _WIN32
	fseek(unvv, diff, SEEK_SET);
#endif
	FILE* out = fopen(outnm, "wb");
	if (out == NULL) {
		puts("Error: could not open file");
		return 1;
	}
  unsigned char* buf = malloc(bufsiz);
  unsigned char* res = malloc(bufsiz>>3);
  if(buf==NULL||res==NULL){
    puts("Error: failed to allocate memory for the 5IGIfier. Reduce buffer size and recompile.");
    goto clean;
  }
	while ((cr = fread(buf, 1, bufsiz>>3, unvv)) > 0) {
		for (size_t i = 0; i < cr >> 3; i++) {
			register unsigned char b = '\0';
			for (signed char j = 7; j >= 0; j--)
				if (buf[7 - j + (i << 3)] == '\'')b |= (1 << j);
			res[i] = b;
		}
		fwrite(res, 1, cr >> 3, out);
	}
	fclose(out);
clean:;
  if(buf!=NULL)
    free(buf);
  if(res!=NULL)
    free(res);
  buf=NULL;
  res=NULL;
	fclose(unvv);
  if(opt!='r'){
    free(encfilnm);
    encfilnm = NULL;
  }
	return 0;
}
int main(int argc, char** argv) {
	clock_t t = clock();
	int res;
	if (argc == 4) {
		if (argv[1][0] == 'e') res = enc(argv[2], argv[3]);
		else if (argv[1][0] == 'd') res = dec('d', argv[2], argv[3]);
    else if(argv[1][0] == 'r') { printf("Decoded raw violeur data has been output at %s", argv[3]); res = dec('r', argv[2], argv[3]);}
		else goto usage;
	}
	else if (argc == 3) {
		if (argv[1][0] == 'd')res = dec('d', argv[2], NULL);
    else if (argv[1][0]=='e')res = enc(argv[2],NULL);
		else if (argv[1][0] == 'f') { puts("Encoded file name: "); dec('f', argv[2], NULL); res = 0; }
		else if (argv[1][0] == 'u') { printf("Time of violing: %" PRId32 "\n", dec('u', argv[2], NULL)); res = 0; }
		else goto usage;
	}
	else goto usage;
	printf("Finished in %fs\n", (double)(clock() - t) / CLOCKS_PER_SEC);
	return res;
usage: {
	puts("Usage: ./(this file's name) d(ecode)/e(ncode)/f(ilename)/r(aw violed data)/u(nixtime) (input file name) (output file name). Omitting the output file name on decode mode will use the file name that was encoded into the input file."); return 4;
	}
}
