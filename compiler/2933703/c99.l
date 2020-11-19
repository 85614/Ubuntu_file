D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P                       ([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS                      ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%{
#include <stdio.h>
#include "c99.tab.h"

void count(void);
void comment(void);
int check_type(void);
%}

%%
"/*"			{ comment(); }
"//"[^\n]*              { /* consume //-comment */ }


"auto"			{ count(); return(AUTO); }
"_Bool"			{ count(); return(BOOL); }
"break"			{ count(); return(BREAK); }
"case"			{ count(); return(CASE); }
"char"			{ count(); return(CHAR); }
"_Complex"		{ count(); return(COMPLEX); }
"const"			{ count(); return(CONST); }
"continue"		{ count(); return(CONTINUE); }
"default"		{ count(); return(DEFAULT); }
"do"			{ count(); return(DO); }
"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE); }
"enum"			{ count(); return(ENUM); }
"extern"		{ count(); return(EXTERN); }
"float"			{ count(); return(FLOAT); }
"for"			{ count(); return(FOR); }
"goto"			{ count(); return(GOTO); }
"if"			{ count(); return(IF); }
"_Imaginary"		{ count(); return(IMAGINARY); }
"inline"		{ count(); return(INLINE); }
"int"			{ count(); return(INT); }
"long"			{ count(); return(LONG); }
"register"		{ count(); return(REGISTER); }
"restrict"		{ count(); return(RESTRICT); }
"return"		{ count(); return(RETURN); }
"short"			{ count(); return(SHORT); }
"signed"		{ count(); return(SIGNED); }
"sizeof"		{ count(); return(SIZEOF); }
"static"		{ count(); return(STATIC); }
"struct"		{ count(); return(STRUCT); }
"switch"		{ count(); return(SWITCH); }
"typedef"		{ count(); return(TYPEDEF); }
"union"			{ count(); return(UNION); }
"unsigned"		{ count(); return(UNSIGNED); }
"void"			{ count(); return(VOID); }
"volatile"		{ count(); return(VOLATILE); }
"while"			{ count(); return(WHILE); }

{L}({L}|{D})*		{ count(); return(check_type()); }

0[xX]{H}+{IS}?		{ count(); return(CONSTANT); }
0[0-7]*{IS}?		{ count(); return(CONSTANT); }
[1-9]{D}*{IS}?		{ count(); return(CONSTANT); }
L?'(\\.|[^\\'\n])+'	{ count(); return(CONSTANT); }

{D}+{E}{FS}?		{ count(); return(CONSTANT); }
{D}*"."{D}+{E}?{FS}?	{ count(); return(CONSTANT); }
{D}+"."{D}*{E}?{FS}?	{ count(); return(CONSTANT); }
0[xX]{H}+{P}{FS}?	{ count(); return(CONSTANT); }
0[xX]{H}*"."{H}+{P}?{FS}?     { count(); return(CONSTANT); }
0[xX]{H}+"."{H}*{P}?{FS}?     { count(); return(CONSTANT); }


L?\"(\\.|[^\\"\n])*\"	{ count(); return(STRING_LITERAL); }

"..."			{ count(); return(ELLIPSIS); }
">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"->"			{ count(); return(PTR_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			{ count(); return(','); }
":"			{ count(); return(':'); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }
"?"			{ count(); return('?'); }

[ \t\v\n\f]		{ count(); }
.			{ /* Add code to complain about unmatched characters */ }

%%

int yywrap(void)
{
	return 1;
}


void comment(void)
{
	char c, prev = 0;
  
	while ((c = input()) != 0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	perror("unterminated comment");
}


int column = 0;

int token_count = 0;

void count(void)
{
	int i;
	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

	ECHO;
}


int check_type(void)
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return TYPE_NAME;
*
*	return IDENTIFIER;
*/

/*
*	it actually will only return IDENTIFIER
*/

	return IDENTIFIER;
}

const char *get_token_str(int c);


int main1 (int argc, char ** argv){
	if (argc>=2){
	  if ((yyin = fopen(argv[1], "r")) == NULL){
	    printf("Can't open file %s\n", argv[1]);
	    return 1;
	  }
	  if (argc>=3){
	    yyout=fopen(argv[2], "w");
	  }
	}
	/* yyin和yyout是lex中定义的输入输出文件指针，它们指明了
	 * lex生成的词法分析器从哪里获得输入和输出到哪里。
	 * 默认：键盘输入，屏幕输出。 
	 */    
	int c = 0;
	printf("%-18s%-22s%-12s\n", "单词", "词素", "属性");
	while (c = yylex()){
		++ token_count;
		printf("%-16s%-20s%-10s\n", get_token_str(c), yytext, "no");
		// printf("type:%d\n", c);
	}
	return 0;
}

const char *get_token_str(int c) {
	switch(c) {
		case IDENTIFIER:
        return "IDENTIFIER";
        break;
    case CONSTANT:
        return "CONSTANT";
        break;
    case STRING_LITERAL:
        return "STRING_LITERAL";
        break;
    case SIZEOF:
        return "SIZEOF";
        break;
    case PTR_OP:
        return "PTR_OP";
        break;
    case INC_OP:
        return "INC_OP";
        break;
    case DEC_OP:
        return "DEC_OP";
        break;
    case LEFT_OP:
        return "LEFT_OP";
        break;
    case RIGHT_OP:
        return "RIGHT_OP";
        break;
    case LE_OP:
        return "LE_OP";
        break;
    case GE_OP:
        return "GE_OP";
        break;
    case EQ_OP:
        return "EQ_OP";
        break;
    case NE_OP:
        return "NE_OP";
        break;
    case AND_OP:
        return "AND_OP";
        break;
    case OR_OP:
        return "OR_OP";
        break;
    case MUL_ASSIGN:
        return "MUL_ASSIGN";
        break;
    case DIV_ASSIGN:
        return "DIV_ASSIGN";
        break;
    case MOD_ASSIGN:
        return "MOD_ASSIGN";
        break;
    case ADD_ASSIGN:
        return "ADD_ASSIGN";
        break;
    case SUB_ASSIGN:
        return "SUB_ASSIGN";
        break;
    case LEFT_ASSIGN:
        return "LEFT_ASSIGN";
        break;
    case RIGHT_ASSIGN:
        return "RIGHT_ASSIGN";
        break;
    case AND_ASSIGN:
        return "AND_ASSIGN";
        break;
    case XOR_ASSIGN:
        return "XOR_ASSIGN";
        break;
    case OR_ASSIGN:
        return "OR_ASSIGN";
        break;
    case TYPE_NAME:
        return "TYPE_NAME";
        break;
    case TYPEDEF:
        return "TYPEDEF";
        break;
    case EXTERN:
        return "EXTERN";
        break;
    case STATIC:
        return "STATIC";
        break;
    case AUTO:
        return "AUTO";
        break;
    case REGISTER:
        return "REGISTER";
        break;
    case INLINE:
        return "INLINE";
        break;
    case RESTRICT:
        return "RESTRICT";
        break;
    case CHAR:
        return "CHAR";
        break;
    case SHORT:
        return "SHORT";
        break;
    case INT:
        return "INT";
        break;
    case LONG:
        return "LONG";
        break;
    case SIGNED:
        return "SIGNED";
        break;
    case UNSIGNED:
        return "UNSIGNED";
        break;
    case FLOAT:
        return "FLOAT";
        break;
    case DOUBLE:
        return "DOUBLE";
        break;
    case CONST:
        return "CONST";
        break;
    case VOLATILE:
        return "VOLATILE";
        break;
    case VOID:
        return "VOID";
        break;
    case BOOL:
        return "BOOL";
        break;
    case COMPLEX:
        return "COMPLEX";
        break;
    case IMAGINARY:
        return "IMAGINARY";
        break;
    case STRUCT:
        return "STRUCT";
        break;
    case UNION:
        return "UNION";
        break;
    case ENUM:
        return "ENUM";
        break;
    case ELLIPSIS:
        return "ELLIPSIS";
        break;
    case CASE:
        return "CASE";
        break;
    case DEFAULT:
        return "DEFAULT";
        break;
    case IF:
        return "IF";
        break;
    case ELSE:
        return "ELSE";
        break;
    case SWITCH:
        return "SWITCH";
        break;
    case WHILE:
        return "WHILE";
        break;
    case DO:
        return "DO";
        break;
    case FOR:
        return "FOR";
        break;
    case GOTO:
        return "GOTO";
        break;
    case CONTINUE:
        return "CONTINUE";
        break;
    case BREAK:
        return "BREAK";
        break;
    case RETURN:
        return "RETURN";
        break;
    default:
        return "SINGLE_sign";
	}
}