#include <stdio.h>
#include <stdlib.h>

char *slurp(char *path)
{
	char *source = NULL;

	FILE *fp = fopen(path, "r");

	if (fp != NULL)
	{
		if (fseek(fp, 0L, SEEK_END) == 0)
		{
			long bufsize = ftell(fp);
			if (bufsize == -1)
			{
				fputs("Error ftell", stderr);
				return NULL;
			}

			source = malloc(sizeof(char) * (bufsize + 1));
			if (source == NULL)
			{
				fputs("Error malloc", stderr);
				return NULL;
			}

			if (fseek(fp, 0L, SEEK_SET) != 0)
			{
				fputs("Error fseek", stderr);
				return NULL;
			}

			size_t newLen = fread(source, sizeof(char), bufsize, fp);
			if (newLen == 0)
			{
				fputs("Error fread", stderr);
				return NULL;
			}
			else
			{
				source[++newLen] = '\0';
			}
		}
		else
		{
			fputs("Error fseek", stderr);
			return NULL;
		}
		fclose(fp);
	}
	else
	{
		fputs("Error fopen", stderr);
		return NULL;
	}

	return source;
}

void run(const char *code) {

	unsigned char mem[30000];
	ssize_t proc[256];
	size_t stack[256];

	size_t cp = 0;
	size_t mp = 0;
	size_t sp = 0;
	ssize_t d = 0;

	for (size_t i = 0; i < 30000; i++)
		mem[i] = 0;

	for (size_t i = 0; i < 256; i++)
		proc[i] = -1;

	while (code[cp])
	{
		switch (code[cp])
		{
		case '.':
			putchar(mem[mp]);
			break;
		case ',':
		{
			int ch = getchar();
			if (ch == EOF)
				mem[mp] = 0;
			else
				mem[mp] = ch;
		}
		break;
		case '+':
			if (mem[mp] == 255)
				mem[mp] = 0;
			else
				mem[mp]++;
			break;
		case '-':
			if (mem[mp] == 0)
				mem[mp] = 255;
			else
				mem[mp]--;
			break;
		case '>':
			if (mp == 65535)
				mp = 0;
			else
				mp = mp + 1;
			break;
		case '<':
			if (mp == 0)
				mp = 65535;
			else
				mp = mp - 1;
			break;
		case '[':
			if (mem[mp] == 0)
			{
				d = 1;
				while (code[cp] && d != 0)
				{
					cp++;
					d += (code[cp] == '[') - (code[cp] == ']');
				}
			}
			break;
		case ']':
			if (mem[mp] != 0)
			{
				d = 1;
				while (cp > 0 && d != 0)
				{
					cp--;
					d += (code[cp] == ']') - (code[cp] == '[');
				}
			}
			break;
		case '(':
			proc[mem[mp]] = cp;
			d = 1;
			while (code[cp] && d != 0)
			{
				cp++;
				d += (code[cp] == '(') - (code[cp] == ')');
			}
			break;
		case ')':
			if (sp > 0)
				cp = stack[--sp];
			break;
		case ':':
			if (sp < 256)
				stack[sp++] = cp;
			if (proc[mem[mp]] != -1)
				cp = proc[mem[mp]];
			break;
		case '/':
			cp++;
			if(code[cp]=='/') {
				while (code[cp] && code[cp] != '\n')
					cp++;
			} else if(code[cp]=='*') {
				while (code[cp] && (code[cp] != '*' || code[cp+1]!='/'))
					cp++;				
			}
			break;
		}
		cp++;
	}
}

int main(int argc,char **argv)
{
	
	if(argc!=2) {
		fprintf(stderr,"Syntax: %s filename.pb\n",argv[0]);
		return 1;
	}
	
	char *code = slurp(argv[1]);	
	run(code);
	free(code);
	code=NULL;
			
	return 0;
}
