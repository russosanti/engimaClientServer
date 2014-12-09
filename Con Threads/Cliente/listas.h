#ifndef LISTAS_H
#define LISTAS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tdatols.h"


typedef struct nodo
{
	Tdato_String info;
	struct nodo *sig;
	struct nodo *ant;
}nodos;


typedef struct lista1_s
{
	nodos *cab;
	nodos *actual;
} tlista1_s;

typedef tlista1_s tlista_s;



void crearnodo(nodos **nue,Tdato_String x)
{
	*nue = (nodos*) malloc(sizeof(nodos));
	(*nue)->sig = NULL;
	(*nue)->ant = NULL;
	(*nue)->info = x;
}

/* Crea la Lista Circular Doblemente Encadenada */
void lcrear (tlista_s *l)
{
	l->cab = NULL;
}

/* Verifica si la lista se encuentra vacía */
int lvacia (tlista_s *l)
{
	return (l->cab == NULL);
}

/* Verifica si la lista se encuentra llena */
int lllena (tlista_s *l)
{
	return(0);
	//return (malloc(sizeof(nodos));
}

/* Se posiciona al principio de la lista */
void lppio (tlista_s *l)
{
	l->actual = l->cab;
}

/* Se posiciona en el siguiente item de la lista */
void lsig(tlista_s *l)
{
	l->actual = l->actual->sig;
}

/* Se posiciona en el item anterior de la lista */
void lant(tlista_s *l)
{
	l->actual = l->actual->ant;
}

/* Se posiciona en el último item de la lista */
int lfin(tlista_s *l) {
	return (l->actual == l->cab);
}

/* Muestra la información del item en la posición solicitada */
void linfo (tlista_s *l,Tdato_String *x)
{
	*x=(l->actual->info);
}

/* Modifica la información del item en la posición solicitada */
void lmodificar (tlista_s *l,Tdato_String aux)
{
	l->actual->info=aux;

}

/* Inserta un elemento al principio de la lista */
void linsertarppio(tlista_s *l, Tdato_String x)
{
	nodos *nue;
	crearnodo(&nue,x);
	if(l->cab==NULL){
		l->cab=nue;
		l->cab->sig=l->cab;
		l->cab->ant=l->cab;
	}else{
		nue->sig=l->cab;
		nue->ant=l->cab->ant;
		l->cab->ant->sig=nue;
		l->cab->ant=nue;
		l->cab=nue;
	}
}


/* Inserta un elemento al final de la lista */
void linsertarfin(tlista_s *l, Tdato_String x)
{
	nodos *nue,*aux;
	crearnodo(&nue,x);
	if (l->cab==NULL){
		l->cab=nue;
		l->cab->sig=l->cab;
		l->cab->ant=l->cab;
	}else{
		aux=l->cab->ant;
		nue->sig=l->cab;
		nue->ant=aux;
		aux->sig=nue;
		l->cab->ant=nue;
	}
}

/* Inserta un elemento en forma ordenada en la lista */
void linsertarorden(tlista_s *l, Tdato_String x, char torden){
nodos *nue;
nodos *aux;

crearnodo(&nue,x);
if(l->cab == NULL){
	l->cab = nue;
	l->cab->sig=l->cab;
	l->cab->ant=l->cab;
}else{
	if((torden=='a' && strcasecmp(l->cab->info.clave,x.clave)>0) || (torden=='d' && strcasecmp(l->cab->info.clave,x.clave)<0)){
		aux=l->cab->ant;
		nue->sig=l->cab;
		nue->ant=aux;
		aux->sig=nue;
		l->cab->ant=nue;
		l->cab=nue;
	}else{
		aux=l->cab;
		while((aux->sig!=l->cab) && ((torden=='a' && strcasecmp(aux->sig->info.clave,x.clave)<0) || (torden=='d' && strcasecmp(aux->sig->info.clave,x.clave)>0))){
			aux=aux->sig;
		 }
		 nue->sig=aux->sig;
		 nue->ant=aux;
		 aux->sig->ant=nue;
		 aux->sig=nue;
	}
}
}

/* Elimina el primer elemento de la lista */
void lborrarppio(tlista_s *l)
{
	 nodos *aux;
	 if(l->cab->sig==l->cab){
		aux=l->cab;
		l->cab=NULL;
	 }else{
		aux=l->cab;
		l->cab->ant->sig=l->cab->sig;
		l->cab->sig->ant=l->cab->ant;
		l->cab=l->cab->sig;
	 }
	 free(aux);
}


/* Elimina el último elemento de la lista */
void lborrarfin(tlista_s *l)
{
nodos *aux;

if(l->cab->ant==l->cab){
	aux=l->cab;
	l->cab=NULL;
}else{
	aux=l->cab->ant;
	aux->ant->sig=l->cab;
	l->cab->ant=aux->ant;
}
free(aux);
}

/* Elimina el elemento de la lista en la posición solicitada */
void lborraractual(tlista_s *l)
{
nodos *aux,*t;

aux=l->actual;
if(l->cab==l->actual){
	 if(l->cab->sig==l->cab){
		l->cab=NULL;
		l->actual=NULL;
	 }else{
		aux=l->cab;
		l->cab->ant->sig=l->cab->sig;
		l->cab->sig->ant=l->cab->ant;
		l->cab=l->cab->sig;
		l->actual=l->cab;
	 }
}else{
	aux=l->actual;
	aux->ant->sig=aux->sig;
	aux->sig->ant=aux->ant;
	l->actual=l->actual->sig;
}
free(aux);
}

/* Busca un elemento de la lista que coincide con el solicitado */
void lbuscar(tlista_s *l,Tclave clave,int *existe){
nodos *aux;
*existe=0;
if(l->cab!=NULL){
	aux=l->cab;
	if(l->cab->sig==l->cab){
		if(strcasecmp(l->cab->info.clave,clave)==0){
			*existe=1;
		}
	}else{
		if(strcasecmp(l->cab->info.clave,l->cab->sig->info.clave)<0){
			while((aux->sig!=l->cab) && (strcasecmp(clave,aux->info.clave)>0)){
				aux=aux->sig;
			}
		}else{
			while((aux->sig!=l->cab) && (strcasecmp(clave,aux->info.clave)<0)){
				aux=aux->sig;
			}
		}
		*existe=(strcasecmp(clave,aux->info.clave)==0);
	}
}
if(*existe){
	l->actual=aux;
}
}

#endif
