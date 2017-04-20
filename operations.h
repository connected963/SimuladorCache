//----------------------------------------------------
//Arquivo:
//operations.h
//Autor: <Pedro Augusto Debastiani Sirtoli>
//----------------------------------------------------

int geraPotencia(int numero) {
  int potencia = 0;
  while(pow(2, potencia) < numero) {
    potencia++;
  }
  return potencia;
}

void buscaDadosEntrada(ParametrosEntrada *parametrosEntrada) {
  FILE *entrada = NULL;
  char dadoEntrada[100];

  if((entrada = fopen("ParametrosEntrada.txt", "rt")) != NULL) {

    while(fscanf(entrada, "%s", dadoEntrada) != EOF) {

      if(strcmp(dadoEntrada, "politicaEscrita:") == 0) {
        fscanf(entrada, "%s", dadoEntrada);
        parametrosEntrada->politicaEscrita = atoi(dadoEntrada);
      }
      else if(strcmp(dadoEntrada, "tamanhoLinha:") == 0) {
        fscanf(entrada, "%s", dadoEntrada);
        parametrosEntrada->tamanhoLinha = atoi(dadoEntrada);
      }
      else if(strcmp(dadoEntrada, "numeroLinhas:") == 0) {
        fscanf(entrada, "%s", dadoEntrada);
        parametrosEntrada->numeroLinhas = atoi(dadoEntrada);
      }
      else if(strcmp(dadoEntrada, "associatividadePorConjunto:") == 0) {
        fscanf(entrada, "%s", dadoEntrada);
        parametrosEntrada->associatividadePorConjunto = atoi(dadoEntrada);
      }
      else if(strcmp(dadoEntrada, "hitTime:") == 0) {
        fscanf(entrada, "%s", dadoEntrada);
        parametrosEntrada->hitTime = atoi(dadoEntrada);
      }
      else if(strcmp(dadoEntrada, "politicaSubstituicao:") == 0) {
        fscanf(entrada, "%s", parametrosEntrada->politicaSubstituicao);
      }
      else if(strcmp(dadoEntrada, "arquivoEntrada:") == 0) {
        fscanf(entrada, "%s", parametrosEntrada->arquivoEntrada);
      }
      else if(strcmp(dadoEntrada, "arquivoSaida:") == 0) {
        fscanf(entrada, "%s", parametrosEntrada->arquivoSaida);
      }
      else if(strcmp(dadoEntrada, "tempoAcessoMP:") == 0) {
        fscanf(entrada, "%s", dadoEntrada);
        parametrosEntrada->tempoAcessoMP = atoi(dadoEntrada);
      }
    }
    parametrosEntrada->palavra = geraPotencia(parametrosEntrada->tamanhoLinha);
    parametrosEntrada->rotulo = 32 - parametrosEntrada->palavra - geraPotencia(parametrosEntrada->numeroLinhas / parametrosEntrada->associatividadePorConjunto);
  }

  //Escreve os dados da cache na tela apenas para facilitar nos testes
  printf("rotulo -> %d\nconjunto -> %d\npalavra -> %d\n", parametrosEntrada->rotulo, geraPotencia(parametrosEntrada->numeroLinhas / parametrosEntrada->associatividadePorConjunto), parametrosEntrada->palavra);
  fclose(entrada);
}

/*
* Função responsável por manter a variável de controle da politica de substituição atualizada.
*/
void atualizaCountCache(Cache **cache, unsigned int line, unsigned int column, char politicaSubstituicao[], int hit) {

  if(hit) {
    if(strcmp(politicaSubstituicao, "LRU") == 0) {
      cache[line][column].count = 0;
    }
    else if(strcmp(politicaSubstituicao, "LFU") == 0) {
      cache[line][column].count++;
    }
  } else {
    cache[line][column].count--;
  }

}

/*
* Realiza busca do bloco "menosUtilizado" (a definição de menos utilizado varia de acordo com o algoritmo de substituição)
*/

int buscaBlocoSubstituicao(Cache **cache,ParametrosEntrada *parametrosEntrada, unsigned int line) {
  int menosUtilizado = 0;

  for(int i = 0; i < parametrosEntrada->associatividadePorConjunto; i++) {
    if(cache[line][i].count < menosUtilizado) {
      menosUtilizado = i;
    }
  }



  return menosUtilizado;
}

/*
 * Gera um index randomico para efetuar a remoção em caches com tipo de politica de substituição aleatorio.
*/
int geraColunaRandomica(ParametrosEntrada *parametrosEntrada) {
  srand((unsigned)time(NULL));
  return rand() % parametrosEntrada->associatividadePorConjunto;
}

/*
* Efetua efetivamente a troca do bloco da cache
*/
void substitui(Cache **cache, ParametrosEntrada *parametrosEntrada, unsigned int data, unsigned int line, int *escritas) {
  int columnSubstituicao = 0;

  if(strcmp(parametrosEntrada->politicaSubstituicao, "LRU") == 0 || strcmp(parametrosEntrada->politicaSubstituicao, "LFU") == 0) {

    columnSubstituicao = buscaBlocoSubstituicao(cache, parametrosEntrada, line);

    if(parametrosEntrada->politicaEscrita && cache[line][columnSubstituicao].writeBack == 1) {
      //escreve na mp
      *escritas = *escritas + 1;
    }

    cache[line][columnSubstituicao].rotulo = data >> (31 - parametrosEntrada->rotulo);
    cache[line][columnSubstituicao].count = 0;
    cache[line][columnSubstituicao].writeBack = 0;

  } else {

    columnSubstituicao = geraColunaRandomica(parametrosEntrada);

    if(parametrosEntrada->politicaEscrita && cache[line][columnSubstituicao].writeBack == 1) {
      //escreve na mp
      *escritas = *escritas + 1;
    }

    cache[line][columnSubstituicao].rotulo = data >> (31 - parametrosEntrada->rotulo);
    cache[line][columnSubstituicao].count = 0;
    cache[line][columnSubstituicao].writeBack = 0;

  }
}

/*
* Efetua a inserção de um bloco na cache
*/
void buscaBloco(Cache **cache, ParametrosEntrada *parametrosEntrada, unsigned int data, unsigned int line, int *escritas) {
  int inserido = 0;

  for(int i = 0; i < parametrosEntrada->associatividadePorConjunto; i++) {
    if(cache[line][i].rotulo == -1) {
      cache[line][i].rotulo = data >> (31 - parametrosEntrada->rotulo);
      cache[line][i].count = 0;
      inserido = 1;
      break;
    }
  }

  if(!inserido) {
    substitui(cache, parametrosEntrada, data, line, escritas);
  }

}

/*
* Realiza as operações de leitura e escrita na cache
*/
int readWrite(Cache **cache, ParametrosEntrada *parametrosEntrada, unsigned int data, int operation, int *leituras, int *escritas) {
  int hit = 0, column = 0;
  unsigned int line = data << parametrosEntrada->rotulo;
  line >>= parametrosEntrada->rotulo;
  line >>= parametrosEntrada->palavra;

  for(int i = 0; i < parametrosEntrada->associatividadePorConjunto; i++) {

    if(cache[line][i].rotulo == data >> (31 - parametrosEntrada->rotulo)) {
      hit = 1;
      column = i;
    }

    atualizaCountCache(cache, line, i, parametrosEntrada->politicaSubstituicao, hit);
  }

  if(!hit) {
    //leitura na mp
    *leituras = *leituras + 1;
    buscaBloco(cache, parametrosEntrada, data, line, escritas);
  }

  //escrita
  if(operation) {
    if(!parametrosEntrada->politicaEscrita) {
      //escreve na mp
      *escritas = *escritas + 1;
    } else {
      cache[line][column].writeBack = 1;
    }
  }

  return hit;
}


/*
* Realiza o armazenamento de todos os registros com writeBack ativo.
*/
void gravaNaMp(Cache **cache, ParametrosEntrada *parametrosEntrada, int *escritas) {

  for(int i = 0; i < parametrosEntrada->numeroLinhas; i++) {
    for(int j = 0; j < parametrosEntrada->associatividadePorConjunto; j++) {
      if(cache[i][j].writeBack == 1) {
        *escritas = *escritas + 1;
        cache[i][j].writeBack = 0;
      }
    }
  }
}

void gravaSaida(int leiturasCache, int escritasCache, int hitLeitura, int hitEscrita, int leiturasMP, int escritasMP, ParametrosEntrada *parametrosEntrada) {

  FILE *fileSaida = NULL;
  double taxaAcertoCache = (double)((hitLeitura+hitEscrita)*100)/(leiturasCache+escritasCache);

  if((fileSaida = fopen(parametrosEntrada->arquivoSaida, "wt")) != NULL) {

    fprintf(fileSaida, "Politica de escrita: %d\n", parametrosEntrada->politicaEscrita);
    fprintf(fileSaida, "Tamanho da linha: %d\n", parametrosEntrada->tamanhoLinha);
    fprintf(fileSaida, "Numero de linhas: %d\n", parametrosEntrada->numeroLinhas);
    fprintf(fileSaida, "Associatividade por conjunto: %d\n", parametrosEntrada->associatividadePorConjunto);
    fprintf(fileSaida, "Tempo de acesso quando encontra: %d\n", parametrosEntrada->hitTime);
    fprintf(fileSaida, "Tamanho da palavra: %d\n", parametrosEntrada->palavra);
    fprintf(fileSaida, "Tamanho do rotulo: %d\n", parametrosEntrada->rotulo);
    fprintf(fileSaida, "Politica de substituição: %s\n", parametrosEntrada->politicaSubstituicao);
    fprintf(fileSaida, "Arquivo de entrada: %s\n", parametrosEntrada->arquivoEntrada);
    fprintf(fileSaida, "Arquivo de saida: %s\n", parametrosEntrada->arquivoSaida);
    fprintf(fileSaida, "Tempo de acesso a memoria principal: %d\n\n", parametrosEntrada->tempoAcessoMP);

    //Arquivo
    fprintf(fileSaida, "Arquivo:\n");
    fprintf(fileSaida, "Total de linhas: %d\n", leiturasCache + escritasCache);
    fprintf(fileSaida, "Total de leituras: %d\n", leiturasCache);
    fprintf(fileSaida, "Total de escritas: %d\n\n", escritasCache);

    //Cache
    fprintf(fileSaida, "Cache:\n");
    fprintf(fileSaida, "Total de leituras: %d\n", leiturasCache);
    fprintf(fileSaida, "Total de acertos em leitura: %d\n", hitLeitura);
    fprintf(fileSaida, "Taxa de acerto em leitura: %.4lf\n\n", (double)(hitLeitura*100)/leiturasCache);

    fprintf(fileSaida, "Total de escrita: %d\n", escritasCache);
    fprintf(fileSaida, "Total de acertos em escrita: %d\n", hitEscrita);
    fprintf(fileSaida, "Taxa de acerto em escrita: %.4lf\n\n", (double)(hitEscrita*100)/escritasCache);

    fprintf(fileSaida, "Taxa de acerto cache: %.4lf\n", (double)((hitLeitura+hitEscrita)*100)/(leiturasCache+escritasCache));
    fprintf(fileSaida, "Tempo médio: %.4lf\n\n", parametrosEntrada->hitTime + (1 - (double)(hitLeitura+hitEscrita)/(leiturasCache+escritasCache)) * parametrosEntrada->tempoAcessoMP);

    //Memória principal
    fprintf(fileSaida, "Memória principal:\n");
    fprintf(fileSaida, "leituras: %d\n", leiturasMP);
    fprintf(fileSaida, "escritas: %d\n", escritasMP);
    fprintf(fileSaida, "acessos: %d\n\n", leiturasMP + escritasMP);


    fclose(fileSaida);

    printf("Os resultados obtidos foram salvos!\n");

  } else {
    printf("Não foi possível gravar os resultados obtidos!\n");
  }

}

void executaSimulacao(Cache **cache, ParametrosEntrada *parametrosEntrada) {
  unsigned int auxRead = 0;
  int leiturasMP = 0, escritasMP = 0, leiturasCache = 0, escritasCache = 0, hitLeitura = 0, hitEscrita = 0;
  char operation = ' ', auxOperation[10];
  FILE *inputData = NULL;

  if((inputData = fopen(parametrosEntrada->arquivoEntrada, "rt")) != NULL) {

    while(fscanf(inputData, "%x", &auxRead) != EOF) {
      fscanf(inputData, "%s", auxOperation);
      operation = auxOperation[0];

      if(operation == 'R') {
        leiturasCache++;
        hitLeitura += readWrite(cache, parametrosEntrada, auxRead, 0, &leiturasMP, &escritasMP);
      } else {
        escritasCache++;
        hitEscrita += readWrite(cache, parametrosEntrada, auxRead, 1, &leiturasMP, &escritasMP);
      }
    }

    if(parametrosEntrada->politicaEscrita) {
      gravaNaMp(cache, parametrosEntrada, &escritasMP);
    }
  }

  printf("Arquivo:\nTotal de linhas: %d\nTotal de leituras: %d\nTotal de escritas: %d\n\n", leiturasCache + escritasCache, leiturasCache, escritasCache);

  printf("Cache:\nTotal de leituras: %d\nTotal de acertos em leitura: %d\nTaxa de acerto em leitura: %.4lf\n\nTotal de escrita: %d\nTotal de acertos em escrita: %d\nTaxa de acerto em escrita: %.4lf\n\nTaxa de acerto cache: %.4lf\nTempo médio: %.4lf\n\n",
              leiturasCache, hitLeitura, (double)(hitLeitura*100)/leiturasCache, escritasCache, hitEscrita, (double)(hitEscrita*100)/escritasCache, (double)((hitLeitura+hitEscrita)*100)/(leiturasCache+escritasCache), (double)parametrosEntrada->hitTime + (1 - (double)(hitLeitura+hitEscrita)/(leiturasCache+escritasCache)) * parametrosEntrada->tempoAcessoMP);

  printf("Memória principal:\nleituras: %d\nescritas: %d\nacessos: %d\n\n", leiturasMP, escritasMP, leiturasMP + escritasMP);

  gravaSaida(leiturasCache, escritasCache, hitLeitura, hitEscrita, leiturasMP, escritasMP, parametrosEntrada);

}
