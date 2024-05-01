/*
 * Name:        wcar.c
 * Description: Word correction and recommendation.
 * Author:      cosh.cage#hotmail.com
 * File ID:     0430240345B0430240600L00173
 * License:     Public Domain.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "svstring.h"
#include "svtree.h"

#define BLOCK_SIZE 32

#define MIN(a, b) ((a) > (b) ? (b) : (a))

typedef struct st_CORRECTION
{
	size_t s;
	char * pstr;
} CORRECTION, * P_CORRECTION;

int _grpCBFCompareInteger(const void * px, const void * py);

size_t WagnerFischer(P_MATRIX pdp, const char * str1, const char * str2)
{
	size_t i = 0, j;
	size_t l1 = strlen(str1);
	size_t l2 = strlen(str2);

	if (0 == l1)
		return l2;
	if (0 == l2)
		return l1;

	for (i = 0; i <= l1; ++i)
		strSetValueMatrix(pdp, i, 0, &i, sizeof(size_t));

	for (j = 0; j <= l2; ++j)
		strSetValueMatrix(pdp, 0, j, &j, sizeof(size_t));

	for (i = 1; i <= l1; ++i)
	{
		char ch1 = str1[i - 1];
		for (j = 1; j <= l2; ++j)
		{
			char ch2 = str2[j - 1];

			size_t k = ch1 == ch2 ? 0 : 1;

			size_t m, n;

			strGetValueMatrix(&m, pdp, i - 1, j, sizeof(size_t));
			++m;
			strGetValueMatrix(&n, pdp, i, j - 1, sizeof(size_t));
			++n;
			m = MIN(m, n);
			strGetValueMatrix(&n, pdp, i - 1, j - 1, sizeof(size_t));
			n += k;
			m = MIN(m, n);

			strSetValueMatrix(pdp, i, j, &m, sizeof(size_t));
		}
	}
		
	strGetValueMatrix(&i, pdp, l1, l2, sizeof(size_t));

	return i;
}

int cbfcmpchar(const void * px, const void * py)
{
	return *(char *)px - *(char *)py;
}

void PrintUsage(void)
{
	printf("wcar word [editorial_distance]\n");
	printf("\tEditorial distance can be an integer. Default value is 1.\n");
}

int main(int argc, char ** argv)
{
	size_t s = 1;

	FILE * fp;

	if (argc > 3 || argc <= 1)
	{
		printf("Bad argument.\n");
		PrintUsage();
		return 0;
	}
	if (3 == argc)
		s = atoi(argv[2]);

	if (NULL != (fp = fopen("words.txt", "r")))
	{
		int c;
		size_t i = 0, j = 0, k = 0;
		char buffer[BLOCK_SIZE] = { 0 };

		TRIE_A pt;
		ARRAY_Z arrword, arrcorr;
		MATRIX mat;
		size_t * psizt;

		treInitTrieA(&pt);
		strInitArrayZ(&arrword, BUFSIZ, sizeof(buffer));
		strInitArrayZ(&arrcorr, BUFSIZ, sizeof(CORRECTION));
		strInitMatrix(&mat, sizeof(buffer), sizeof(buffer), sizeof(size_t));

		while (EOF != (c = fgetc(fp)))
		{
			switch (c)
			{
			case '\n':
			case '\r':
				if (k < i)
					k = i;

				treInsertTrieA(&pt, buffer, i, sizeof(char), (size_t)strLocateItemArrayZ(&arrword, sizeof(buffer), j), cbfcmpchar);

				i = 0;

				strcpy((char *)strLocateItemArrayZ(&arrword, sizeof(buffer), j), buffer);
				
				++j;

				if (j >= strLevelArrayZ(&arrword))
					strResizeArrayZ(&arrword, strLevelArrayZ(&arrword) + BUFSIZ, sizeof(buffer));
				
				break;
			default:
				buffer[i++] = (char)c;
				buffer[i] = '\0';
			}
		}
		strResizeArrayZ(&arrword, j, sizeof(buffer));

		if (NULL == (psizt = treSearchTrieA(&pt, argv[1], strlen(argv[1]), sizeof(char), cbfcmpchar)))
		{
			for (j = i = 0; i < strLevelArrayZ(&arrword); ++i)
			{
				if ((k = WagnerFischer(&mat, (const char *)strLocateItemArrayZ(&arrword, sizeof(buffer), i), argv[1])) <= s)
				{
					((P_CORRECTION)strLocateItemArrayZ(&arrcorr, sizeof(CORRECTION), j))->pstr = (char *)strLocateItemArrayZ(&arrword, sizeof(buffer), i);
					((P_CORRECTION)strLocateItemArrayZ(&arrcorr, sizeof(CORRECTION), j))->s = k;
					if (++j >= strLevelArrayZ(&arrcorr))
						strResizeArrayZ(&arrcorr, strLevelArrayZ(&arrcorr) + BUFSIZ, sizeof(CORRECTION));
				}
			}
			strResizeArrayZ(&arrcorr, j, sizeof(CORRECTION));

			strSortArrayZ(&arrcorr, sizeof(CORRECTION), _grpCBFCompareInteger);

			for (i = 0; i < strLevelArrayZ(&arrcorr); ++i)
				printf("%s\n", ((P_CORRECTION)strLocateItemArrayZ(&arrcorr, sizeof(CORRECTION), i))->pstr);
		}
		
		fclose(fp);
		treFreeTrieA(&pt, sizeof(char));
		strFreeArrayZ(&arrword);
		strFreeArrayZ(&arrcorr);
		strFreeMatrix(&mat);
	}
	else
		fprintf(stderr, "Open database error.\n");

	return 0;
}
