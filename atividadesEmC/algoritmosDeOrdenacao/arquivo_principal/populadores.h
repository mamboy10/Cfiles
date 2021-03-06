/*
 *	Arquivo-cabeçalho de populadores
 *	de vetores.
 *	
 *	
 *
 *
 */
#ifndef HAVE_TIME_H
	#include <time.h>
#endif
#ifndef HAVE_STDLIB_H
	#include <stdlib.h>
#endif

int randomiza_com_limites( int min , int max );

void popula_aleatorio( int N , int *lista );
void popula_crescente( int N , int *lista );
void popula_descrescente( int N , int *lista );
void popula_50_porcento_iguais( int N , int *lista );
void popula_crescente_e_decrescente( int N , int *lista );
void popula_zero( int N , int *lista );

void semeia_rand(void);