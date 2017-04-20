//----------------------------------------------------
//Arquivo:
//main.c
//Autor: <Pedro Augusto Debastiani Sirtoli>
//----------------------------------------------------

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "structure.h"
#include "operations.h"

/*
* Realiza a alocação da memória referente a cache
*/
Cache** criaCache(unsigned int linhas, unsigned int colunas) {

  Cache **cache = (Cache **)malloc(linhas * sizeof(Cache*));

  for(unsigned int i = 0; i < linhas; i++) {
    cache[i] = (Cache *)malloc(colunas * sizeof(Cache));

    for(unsigned int j = 0; j < colunas; j++) {
      cache[i][j].rotulo = -1;
      cache[i][j].count = 0;
      cache[i][j].writeBack = 0;
    }
  }

  return cache;
}

int main() {
  /*
    Realiza busca dos parâmetros antes de criar a cache.
  */
  ParametrosEntrada parametrosEntrada;

  buscaDadosEntrada(&parametrosEntrada);

  Cache **cache = criaCache(parametrosEntrada.numeroLinhas, parametrosEntrada.associatividadePorConjunto);

  executaSimulacao(cache, &parametrosEntrada);
}
