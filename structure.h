//----------------------------------------------------
//Arquivo:
//structure.h
//Autor: <Pedro Augusto Debastiani Sirtoli>
//----------------------------------------------------

typedef struct PARAMETROS_ENTRADA ParametrosEntrada;
struct PARAMETROS_ENTRADA{
  char politicaEscrita;
  int tamanhoLinha;
  int numeroLinhas;
  int associatividadePorConjunto;
  int hitTime;
  int palavra;
  int rotulo;
  char politicaSubstituicao[10];
  char arquivoEntrada[255];
  char arquivoSaida[255];
  int tempoAcessoMP;

};

typedef struct CACHE Cache;
struct CACHE{
  short int rotulo;
  short int writeBack;
  int count;

};
