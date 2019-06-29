#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <limits.h>
#include <stdlib.h>

#include "arvore_b_mais.h"
#include "lista_pizzas.h"
#include "metadados.h"
#include "no_folha.h"
#include "no_interno.h"


int busca(int cod, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados, int d)
{
    //TODO: Inserir aqui o codigo do algoritmo
    FILE *fmetadados = fopen(nome_arquivo_metadados, "rb");
    FILE *findices = fopen(nome_arquivo_indice, "rb");
    FILE *fdados = fopen(nome_arquivo_dados, "rb");
    TMetadados* meta = le_metadados(fmetadados);
    int i;

    //caso a raiz seja folha, a busca retorna o proprio ponteiro da raiz.
    if(meta->raiz_folha)
    {
        return meta->pont_raiz;
    }
    //caso a raiz seja um indice, será preciso procurar o nó onde o registro está/estaria localizado

    //pular para a raiz
    fseek(findices, meta->pont_raiz * tamanho_no_interno(d), SEEK_SET);

    //ler o nó
    TNoInterno* in_node = le_no_interno(d, findices);

    //permanecer buscando até que aponte para a folha onde esta/estaria localizado o registro
    while(in_node->aponta_folha == 0)
    {
        //busca o ponteiro certo do nó
        for(i = 0; i < in_node->m; i++)
        {
            if(cod < in_node->chaves[i]) {
                break;
            }
        }
        int next_ptr = in_node->p[i];

        //pula para o novo nó e realiza a leitura do mesmo.
        fseek(findices, next_ptr, SEEK_SET);
        libera_no_interno(in_node);
        in_node = le_no_interno(d, findices);
    }

    //quando apontar para folha basta pegar o ponteiro final
    for(i = 0; i < in_node->m; i++)
    {
        if(cod <= in_node->chaves[i])
        {
            break;
        }
    }

    int final_ptr = in_node->p[i];

    //libera o que continua na heap
    free(meta);
    libera_no_interno(in_node);

    fclose(findices);
    fclose(fdados);
    fclose(fmetadados);
    return final_ptr;

}

int insere(int cod, char *nome, char *categoria, float preco, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados, int d)
{
	//TODO: Inserir aqui o codigo do algoritmo de insercao
    return INT_MAX;
}

int exclui(int cod, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados, int d)
{
	//TODO: Inserir aqui o codigo do algoritmo de remocao
    return INT_MAX;
}

void carrega_dados(int d, char *nome_arquivo_entrada, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados)
{
    //TODO: Implementar essa funcao
}