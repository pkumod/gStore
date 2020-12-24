/*=============================================================================
# Filename:		temp.c
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified:	2015-05-12 15:40
# Description: 
=============================================================================*/

#include <stdio.h>

int
main(int argc, char* argv[])
{
  int i = 0;
  char c = 'A';
  FILE* fp = fopen("a.dat", "w+b");
  fwrite(&c, sizeof(char), 1, fp);
  //when moving to tail, always return 0(point to tail whne exceeding)
  //when moving to head, -1 when exceeding(point to original)
  fseek(fp, 4096 * 10, SEEK_SET);
  //file hole, diffrent across platforms
  //mv just change index, so not vary, cp may vary(if transfer data, not neglect hole)
  //compress/uncompress will change the size with du -h
  //du and ll is different: one for block usage(also neglect hole),
  //one for actual size(not neglect hole)
  //what's more, even after cp or compress/uncompress, there may also be holes.
  fwrite(&i, sizeof(int), 1, fp);
  fclose(fp);
  /*
	FILE* fp = fopen("data.dat", "r+");
	char c;
	while((c = fgetc(fp)) != EOF)
		printf("%c", c);
	fclose(fp);
	printf("\n");
	*/
  /*
	printf("%c%c%c", 0xE6, 0x88, 0x91);
	printf("%c%c%c", 0xE7, 0x88, 0xB1);
	printf("%c%c%c", 0xE5, 0x90, 0xB4);
	printf("%c%c%c", 0xE5, 0x98, 0x89);
	printf("%c%c%c\n", 0xE5, 0x8D, 0xBF);
	*/
  return 0;
}
