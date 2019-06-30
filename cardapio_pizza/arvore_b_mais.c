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

//funcoes auxiliares
void sort_no_folha(TNoFolha *noFolha){
    int min;
    TPizza *aux;
    for (int i=0; i< noFolha->m-1; i++){
        min = i;
        for (int j=(i+1); j<noFolha->m; j++){
            if (noFolha->pizzas[j]->cod< noFolha->pizzas[i]->cod){
                min = j;
            }
        }
        if (noFolha->pizzas[min]->cod!=noFolha->pizzas[i]->cod){
            aux = noFolha->pizzas[i];
            noFolha->pizzas[i] = noFolha->pizzas[min];
            noFolha->pizzas[min] = aux;
        }
    }
}


TNoFolha * particiona_folha(int d, TNoFolha *antiga){
    TNoFolha *novo = no_folha(d, d+1, antiga->pont_pai, antiga->pont_prox);
    int pos=0;
    for (int i=d; i<antiga->m; i++){
        novo->pizzas[pos] = antiga->pizzas[i];
        antiga->pizzas[i]=NULL;
        pos++;
    }
    antiga->m = d;
    return novo;

}

void sort_no_interno(int d, TNoInterno *node){
    int min;
    int aux;
    for (int i=0; i< node->m-1; i++){
        min = i;
        for (int j=(i+1); j<node->m; j++){
            if (node->chaves[j] < node->chaves[min]){
                min = j;
            }
        }
        if (node->chaves[min]!=node->chaves[i]){
            aux = node->chaves[i];
            node->chaves[i] = node->chaves[min];
            node->chaves[min] = aux;

            //acerta os ponteiros
            int aux_pont = node->p[i+1];
            node->p[i+1] = node->p[min+1];
            node->p[min+1] = aux_pont;
        }
    }

}

//funcoes da arvore b+
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
    //transforma as informacoes cod, nome, desc em Cliente, mudar esse null
    TPizza* nova_pizza = pizza(cod, nome, categoria, preco);

    //abrir arquivo de folhas
    FILE* metadados_file = fopen(nome_arquivo_metadados, "rb+");
    TMetadados* meta = le_metadados(metadados_file);
    FILE *indice_file = fopen(nome_arquivo_indice,"rb+");
    FILE *dados_file = fopen(nome_arquivo_dados, "rb+");

    //pular para a raiz e ler o no folha
    int folha_ptr = busca(cod, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);
    fseek(dados_file, folha_ptr, SEEK_SET);
    TNoFolha* folha_node = le_no_folha(d, dados_file);

    int inserido_ptr;
    int taNaArvore = 0;
    for (int i=0; i<folha_node->m; i++){
        if (folha_node->pizzas[i]->cod==cod) taNaArvore=1;
    }
    //se ja existir esse codigo, retorna -1
    if (taNaArvore) return -1;
    else {
        //tem espaço para inserir na folha
        if (folha_node->m < 2 * d) {
            //insere a informacao na ultima posicao da folha e ordena
            folha_node->pizzas[folha_node->m] = nova_pizza;
            folha_node->m += 1;
            sort_no_folha(folha_node);

            //sobrescreve a folha
            fseek(dados_file, folha_ptr, SEEK_SET);
            salva_no_folha(d, folha_node, dados_file);
            inserido_ptr = folha_ptr;
            //fclose(dados_file);
            //return folha_ptr;
        }
        //caso a folha esteja cheia, particionar
        else {
            int cod_indice;
            cod_indice = folha_node->pizzas[0]->cod;
            folha_node->pizzas[folha_node->m] = nova_pizza;
            folha_node->m += 1;
            sort_no_folha(folha_node);
            TNoFolha *novoNo = particiona_folha(d, folha_node);
            int pont_new;
            //pega a posicao que vai entrar o novo no
            fseek(dados_file, 0, SEEK_END);
            pont_new = ftell(dados_file);
            //salva o no novo
            novoNo->pont_prox = folha_node->pont_prox;
            folha_node->pont_prox = pont_new;
            salva_no_folha(d, novoNo, dados_file);
            //subscreve o no antigo
            fseek(dados_file, folha_ptr, SEEK_SET);
            salva_no_folha(d, folha_node, dados_file);


            //se o primeiro nó continuar sendo a chave, cria uma nova chave para inserir no no interno
            fseek(indice_file, folha_node->pont_pai, SEEK_SET);
            TNoInterno *in_node = le_no_interno(d, indice_file);
            //se o no nao estiver cheio
            if (folha_node->pont_pai != -1) {
                if (in_node->m < 2 * d) {
                    //se o primeiro no da folha nao tiver mudado
                    if (folha_node->pizzas[0]->cod == cod_indice) {
                        in_node->chaves[in_node->m] = novoNo->pizzas[0]->cod;
                        in_node->p[in_node->m + 1] = pont_new;
                        in_node->m+=1;
                        sort_no_interno(d, in_node);

                        int tanonovo = 0;
                        for (int i = 0; i < novoNo->m; i++) {
                            if (novoNo->pizzas[i]->cod == cod) {
                                tanonovo = 1;
                                break;
                            }
                        }
                        if (tanonovo) inserido_ptr = pont_new;
                        else inserido_ptr = folha_ptr;

                        //atualiza os indices
                        fseek(indice_file, folha_node->pont_pai, SEEK_SET);
                        salva_no_interno(d, in_node, indice_file);

                        //atualiza os metadados
                        fseek(dados_file, 0, SEEK_END);
                        meta->pont_prox_no_folha_livre = ftell(dados_file);
                        fseek(metadados_file, 0, SEEK_SET);
                        salva_metadados(meta, metadados_file);
                    }
                }
            }
        }
        /*
        int new_size = d+1;
        TNoFolha* nova_folha_node = cria_no_folha(d, folha_node->pont_pai, folha_node->pont_prox, new_size, nova_pizza);

        //separar os dados entre as 2 folhas(fazer uma c com (2*d + 1) pizzas,
        // adicionar todos os da folha e mais o novo, ordenar por codigo e separar entre as 2 folhas)

        //pegar a chave que vai subir para o no interno
        int chave_nova_pizza = nova_folha_node->pizzas[0]->cod;

        //ir ate o final do arquivo de dados para pegar o novo ponteiro
        fseek(dados_file, 0, SEEK_END);
        int new_ptr = ftell(dados_file);

        //salva a nova folha
        salva_no_folha(d, nova_folha_node, dados_file);
        //sobrescreve a folha antiga
        fseek(dados_file, folha_ptr, SEEK_SET);
        salva_no_folha(d, folha_node, dados_file);

        //agora é preciso alterar os arquivos de indice
        //caso a raiz fosse folha, o arquivo de indices estarão vazio
        if(meta->raiz_folha){
            //1 indice apenas
            int m = 1;
            //pont_pai = -1 ja q ele vai ser a raiz
            int pont_pai = -1;
            int aponta_folha = 1;
            TNoInterno* in_node = no_interno(d, m, pont_pai, aponta_folha);
            in_node->p[0] = folha_ptr;
            in_node->p[1] = new_ptr;
            in_node->chaves[0] = chave_nova_pizza;

            //escrever no final do arquivo e pegar o ponteiro
            fseek(indice_file, 0, SEEK_END);
            int root_ptr = ftell(indice_file);
            salva_no_interno(d, in_node, indice_file);
            meta->raiz_folha = 0;
            meta->pont_raiz = root_ptr;

            //reescrever metadados
            fseek(metadados_file, 0, SEEK_SET);
            salva_metadados(meta, metadados_file);
        }

        fseek(indice_file, folha_node->pont_pai, SEEK_SET);
        TNoInterno* in_node = le_no_interno(d, indice_file);

        int i = 0;

        //enquanto os nos internos estiverem cheios, particionar eles
        while(in_node->m == 2*d){
            //Qntd de elementos da nova pagina
            //Achando o pai para particionar
            int pai_ptr = busca(in_node->pont_pai,nome_arquivo_metadados,nome_arquivo_indice,nome_arquivo_dados,d);
            fseek(indice_file, folha_ptr * tamanho_no_interno(d), SEEK_SET);
            TNoInterno* new_node = le_no_interno(d,indice_file);

            //Cria um novo Nó
            TNoInterno *new_apontado = cria_no_interno(d,in_node->m,in_node->pont_pai,in_node->aponta_folha,in_node->m/2);

            //"Transferencia de dados da pagina antiga para a nova"
            new_node->aponta_folha = busca(new_apontado->chaves[2*d - i],nome_arquivo_metadados,nome_arquivo_indice,nome_arquivo_dados,d);
            new_apontado->p[i] = in_node->p[2*d - 1];
            i++;

            //Danndo free nos nodes criados para a transferencia
            libera_no_interno(new_node);
            libera_no_interno(new_apontado);

            //Atualizando o arquivo
            salva_no_interno(d,new_node,indice_file);
        }
        int final_ptr = in_node->p[i];
        libera_no_interno(in_node);
        fclose(indice_file);
        return final_ptr;
        */
        fclose(indice_file);
        fclose(dados_file);
        fclose(metadados_file);
        return inserido_ptr;
    }
}

int exclui(int cod, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados, int d)
{
	//TODO: Inserir aqui o codigo do algoritmo de remocao
    FILE* metadados_file = fopen(nome_arquivo_metadados, "rb+");
    TMetadados* meta = le_metadados(metadados_file);
    FILE *indice_file = fopen(nome_arquivo_indice,"rb+");
    FILE *dados_file = fopen(nome_arquivo_dados, "rb+");

    int folha_ptr = busca(cod, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);
    fseek(dados_file, folha_ptr, SEEK_SET);
    TNoFolha *folha_node = le_no_folha(d, dados_file);
    //se nao for necessario concatenar nada
    if (folha_node->m>=d+1) {
        int voltar1 = 0;
        for (int i = 0; i < folha_node->m; i++) {
            if (voltar1 == 1 && i != 0) {
                folha_node->pizzas[i - 1] = folha_node->pizzas[i];
            }
            if (folha_node->pizzas[i]->cod == cod) {
                voltar1 = 1;
            }
            if(voltar1 && i==folha_node->m-1){
                folha_node->pizzas[i]=NULL;
            }
        }
        fseek(dados_file, folha_ptr, SEEK_SET);
        folha_node->m -=1;
        salva_no_folha(d, folha_node, dados_file);
        fclose(dados_file);
        return folha_ptr;
    }


    return INT_MAX;
}

void carrega_dados(int d, char *nome_arquivo_entrada, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados)
{

    //TODO: Implementar essa funcao
}


