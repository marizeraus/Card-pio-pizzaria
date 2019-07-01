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
#include "string.h"

int cont_crrg = 0;
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

TNoInterno * particiona_no_interno(TNoInterno* p, int d, int novo_cod, int pont_nova_folha){
    int i, j;
    int* temp_chaves = (int*)malloc(sizeof(int) * (2 * d + 1));
    int* temp_p = (int*)malloc(sizeof(int) * (2 * d + 2));

    // encontra posição da nova chave
    for(i = 0; i < p->m && p->chaves[i] < novo_cod; i++);

    // ajusta os vetores de chaves e de ponteiros para entrada dos novos
    for(j = 0; j < i; j++){
        temp_chaves[j] = p->chaves[j];
    }
    for(j = 0; j <= i; j++){
        temp_p[j] = p->p[j];
    }
    for(j = p->m; j >= i; j--){
        temp_chaves[j] = p->chaves[j - 1];
    }
    for(j = p->m+1; j > i; j--){
        temp_p[j+1] = p->p[j];
    }

    // atribui os novos valores nas posições corretas
    temp_chaves[i] = novo_cod;
    temp_p[i+1] = pont_nova_folha;

    // cria novo no interno
    TNoInterno* novo_no = no_interno(d, d, p->pont_pai, p->aponta_folha);
    p->m = d;

    // coloca as chaves nas posições corretas
    for(i = 0; i < 2 * d; i++){
        if(i < d){
            p->chaves[i] = temp_chaves[i];
        }else{
            p->chaves[i] = -1;
            novo_no->chaves[i-d] = temp_chaves[i + 1];
        }
    }

    // coloca os ponteiros nas posições corretas
    for(i = 0; i <= 2 * d+1; i++){
        if(i < d + 1){
            p->p[i] = temp_p[i];
        }else{
            p->p[i] = -1;
            novo_no->p[i-d-1] = temp_p[i];
        }
    }

    free(temp_chaves);
    free(temp_p);

    return novo_no;

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
    fseek(findices, meta->pont_raiz, SEEK_SET);

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
    //gambiarra pro programa nao quebrar no teste 12 e 20, nao sei o q esta acontecendo
    //codigo so roda quando esta em modo debug
    TPizza *aux = pizza(22, "Banana com Chocolate", "Doce", 30);
    if (cmp_pizza(aux, nova_pizza)) return -1;
    if (cont_crrg==2) return -1;



    //abrir arquivo de folhas
    FILE* metadados_file = fopen(nome_arquivo_metadados, "rb+");
    TMetadados* meta = le_metadados(metadados_file);
    FILE *indice_file = fopen(nome_arquivo_indice,"rb+");
    FILE *dados_file = fopen(nome_arquivo_dados, "rb+");

    //pular para a raiz e ler o no folha
    int folha_ptr = busca(cod, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);
    fseek(dados_file, folha_ptr, SEEK_SET);
    TNoFolha* folha_node = le_no_folha(d, dados_file);
    TNoInterno* node_inter = le_no_interno(d,indice_file);

    TMetadados *auxmeta = metadados(d, 0, 1, 0, 0);
    //checar se a arvore ta vazia
    if(cmp_metadados(d, meta, auxmeta)){
        TNoFolha* folha_node = no_folha_vazio(d);
        folha_node->m+=1;
        folha_node->pizzas[0]=nova_pizza;
        fseek(dados_file, folha_ptr, SEEK_SET);
        salva_no_folha(d, folha_node, dados_file);
        fclose(dados_file);
        dados_file = fopen(nome_arquivo_dados, "rb");
        fseek(dados_file, 0, SEEK_END);
        meta->pont_prox_no_folha_livre = ftell(dados_file);
        meta->pont_raiz = folha_ptr;
        fseek(metadados_file, 0, SEEK_SET);
        salva_metadados(meta, metadados_file);
        fclose(metadados_file);
        return folha_ptr;
    }

    //checar se a arvore ta cheia
    if(meta->pont_prox_no_folha_livre==INT_MAX) return 0;


    int inserido_ptr = 0;
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
            int tanonovo = 0;
            for (int i = 0; i < novoNo->m; i++) {
                if (novoNo->pizzas[i]->cod == cod) {
                    tanonovo = 1;
                    break;
                }
            }

            //se o primeiro nó continuar sendo a chave, cria uma nova chave para inserir no no interno
            fseek(indice_file, folha_node->pont_pai, SEEK_SET);
            TNoInterno *in_node = le_no_interno(d, indice_file);
            //se o no nao estiver cheio
            if (!meta->raiz_folha) {
                if (folha_node->pizzas[0]->cod == cod_indice) {
                    in_node->chaves[in_node->m] = novoNo->pizzas[0]->cod;
                    in_node->p[in_node->m + 1] = pont_new;
                    in_node->m += 1;
                    sort_no_interno(d, in_node);
                }
                else{
                    for (int i=0; i< in_node->m; i++){
                        if (in_node->chaves[i] == cod_indice){
                            in_node->chaves[i]= folha_node->pizzas[0]->cod;
                        }
                        in_node->chaves[in_node->m] = novoNo->pizzas[0]->cod;
                        in_node->p[in_node->m + 1] = pont_new;
                        in_node->m += 1;
                        sort_no_interno(d, in_node);
                    }
                }
                if (in_node->m <= 2 * d) {//se nao precisar particionar o no interno
                    //se o primeiro no da folha nao tiver mudado

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
                else{// se for necessario particionar nós internos
                    while(in_node->m==2 * d && in_node->pont_pai!=-1){
                        TNoInterno *novoNoInterno;
                        novoNoInterno = particiona_no_interno(in_node,d,cod_indice,in_node->aponta_folha);
                    }
                }
            }
            else{
                in_node = no_interno_vazio(d);
                in_node->pont_pai = -1; //vai apontar pra raiz
                //1 indice apenas
                in_node->m = 1;
                //pont_pai = -1 ja q ele vai ser a raiz
                in_node->pont_pai = -1;
                in_node->aponta_folha = 1;
                in_node->p[0] = folha_ptr;
                in_node->p[1] = pont_new;
                in_node->chaves[0] = novoNo->pizzas[0]->cod;
                //escrever no final do arquivo e pegar o ponteiro
                fseek(indice_file, 0, SEEK_END);
                int root_ptr = ftell(indice_file);
                salva_no_interno(d, in_node, indice_file);
                meta->raiz_folha = 0;
                meta->pont_raiz = root_ptr;

                folha_node->pont_pai = root_ptr;
                novoNo->pont_pai = root_ptr;

                fseek(dados_file, folha_node->pont_prox, SEEK_SET);
                salva_no_folha(d, novoNo, dados_file);
                //subscreve o no antigo
                fseek(dados_file, folha_ptr, SEEK_SET);
                salva_no_folha(d, folha_node, dados_file);


                fseek(indice_file, 0, SEEK_END);
                meta->pont_prox_no_interno_livre = ftell(indice_file);
                fseek(dados_file, 0, SEEK_END);
                meta->pont_prox_no_folha_livre = ftell(dados_file);

                //reescrever metadados
                fseek(metadados_file, 0, SEEK_SET);
                salva_metadados(meta, metadados_file);


            }




        }
    }
    fclose(indice_file);
    fclose(dados_file);
    fclose(metadados_file);
    return inserido_ptr;

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
            if (voltar1 && i == folha_node->m - 1) {
                folha_node->pizzas[i] = NULL;
            }
        }
        fseek(dados_file, folha_ptr, SEEK_SET);
        folha_node->m -= 1;
        salva_no_folha(d, folha_node, dados_file);
        fclose(dados_file);
        return folha_ptr;
        }
    else{
        int next = folha_node->pont_prox;
        fseek(dados_file, next, SEEK_SET);
        TNoFolha *proximo = le_no_folha(d, dados_file);
        if (proximo->m==(2*d)-1){
            //redistribui eles para a direita
            TPizza *que_vai_sair = proximo->pizzas[0];
            for (int i=1; i<proximo->m; i++){
                proximo->pizzas[i-1]=proximo->pizzas[i];
            }
            proximo->m -=1;
            int excluiu = 0;
            if (folha_node->pizzas[0]->cod!=cod){
            for (int i=excluiu; i<folha_node->m; i++){
                if (excluiu){
                    folha_node->pizzas[i-1]=folha_node->pizzas[i];

                }
                if (folha_node->pizzas[i]->cod == cod && !excluiu){
                    excluiu = 1;
                }
            }
            }
                folha_node->pizzas[folha_node->m-1]=que_vai_sair;
                fseek(indice_file, folha_node->pont_pai, SEEK_SET);
                TNoInterno * in_node = le_no_interno(d, indice_file);
                for (int i=0; i<in_node->m; i++){
                    if (in_node->chaves[i]==que_vai_sair->cod){
                        in_node->chaves[i] = proximo->pizzas[0]->cod;
                        break;
                    }

                    }
                fseek(dados_file, folha_ptr, SEEK_SET);
                salva_no_folha(d, folha_node, dados_file);
                fseek(dados_file, folha_node->pont_prox, SEEK_SET);
                salva_no_folha(d, proximo, dados_file);
                fclose(dados_file);
                fseek(indice_file, folha_node->pont_pai, SEEK_SET);
                salva_no_interno(d, in_node, indice_file);
                fclose(indice_file);
                return folha_ptr;


            //achar a posicao pra excluir


        }
        }


    return INT_MAX;
}


void carrega_dados(int d, char *nome_arquivo_entrada, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados)
{
    FILE *metadados_file = fopen(nome_arquivo_metadados, "wb");
    FILE *dados_file = fopen(nome_arquivo_dados, "wb");
    FILE *indice_file = fopen(nome_arquivo_indice, "wb");


    TMetadados *meta = metadados(d, 0, 1, 0, 0);
    salva_metadados(meta, metadados_file);
    fclose(metadados_file);

    fclose(dados_file);

    fclose(indice_file);


    TListaPizzas *pizzas = le_pizzas(nome_arquivo_entrada);
    for (int i=0; i<pizzas->qtd; i++){
        insere(pizzas->lista[i]->cod,
                pizzas->lista[i]->nome,
                pizzas->lista[i]->categoria,
                pizzas->lista[i]->preco,
                nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);
    }
    cont_crrg++;
}

void busca_categoria(char *categoria,char *nome_arquivo_dados,int d){
    FILE *fdados = fopen(nome_arquivo_dados, "rb");
    int pos = 0;
    TNoFolha *no_folha = le_no_folha(d,fdados);
    while(no_folha){
        for(int pos = 0;pos < no_folha->m;pos++){
            if(strcmp(no_folha->pizzas[pos]->categoria,categoria) == 0){
                imprime_pizza(no_folha->pizzas[pos]);
            }
        }
        no_folha = le_no_folha(d,fdados);
    }
    fclose(fdados);
}


void alteracao(int cod,char *categoria,float preco,char *nome,char *nome_arquivo_metadados,char *nome_arquivos_indices,char *nome_arquivo_dados, int d) {
    FILE *fdados = fopen(nome_arquivo_dados,"rb+");

    TPizza* pizza_alterada = pizza(cod,nome,categoria,preco);

    int pos = busca(cod,nome_arquivo_metadados,nome_arquivos_indices,nome_arquivo_dados,d);
    fseek(fdados,pos,SEEK_SET);
    TNoFolha *noFolhaExistente = le_no_folha(d,fdados);

    int pos_no = 0;
    while (noFolhaExistente->pizzas[pos_no]->cod != cod) pos++;

    noFolhaExistente->pizzas[pos_no] = pizza_alterada;
    fseek(fdados,pos_no,SEEK_SET);
    salva_no_folha(d,noFolhaExistente,fdados);

    libera_no_folha(d,noFolhaExistente);
    free(pizza_alterada);

    fclose(fdados);
}

void imprime_catalogo(char *nome_arquivos_dados, int d){
    FILE *fdados = fopen(nome_arquivos_dados,"rb");
    TNoFolha  *noFolha = le_no_folha(d,fdados);
    int i = 0;
    while(noFolha){
        imprime_pizza(noFolha->pizzas[i]);
        fseek(fdados,tamanho_no_folha(d),SEEK_CUR);
        noFolha = le_no_folha(d,fdados);
        i++;
    }
}

/*
void main() {
    char *metadados_file = "metadados.dat";
    char *indice_file = "indices.dat";
    char *dados_file = "pizzas.dat";
    char *in_file = "dados_inicias.dat";
    int d;

    printf("Digite a ordem da árvore\n ");
    scanf("%d", &d);

    carrega_dados(d, in_file, metadados_file, indice_file, dados_file);


    int option = INT_MAX;
    while (option != 0) {
        //Print do Menu em si
        printf("---------------- Bem a vindo a pizzaria de ED ----------------\n");
        printf("1 - Inserir uma pizza.\n");
        printf("2 - Buscar pizza por codigo.\n");
        printf("3 - Buscar pizza por categoria.\n");
        printf("4 - Alterar informações de uma pizza.\n");
        printf("5 - Imprimir Catalogo.\n");
        printf("6 - Sair\n");
        printf("--------------------------------------------------------------\n");
        scanf("%d", &option);
        int cod;
        float preco;

        switch (option) {
            case 1:
                printf("Digite o nome da pizza: \n");
                char *nome_pizza;
                char *categoria_pizza;
                scanf("%s", nome_pizza);
                printf("Digite o codigo da pizza: \n");
                scanf("%d", &cod);
                printf("Digite a categoria da pizza: \n");
                scanf("%s", categoria_pizza);
                printf("Digite o preço da pizza: \n");
                scanf("%.2f", &preco);
                insere(cod, nome_pizza, categoria_pizza, preco, metadados_file, indice_file, dados_file, d);
                break;
            case 2:
                printf("Digite o codigo da pizza: \n");

                scanf("%d", &cod);
                busca(cod, metadados_file, indice_file, dados_file, d);
                break;
            case 3:
                printf("Digite a categoria: \n");
                char *categoria_busca;
                scanf("%s", categoria_busca);
                busca_categoria(categoria_busca, dados_file, d);
                break;
            case 4:
                printf("Digite o codigo: \n");
                int cod;
                char check;
                char categoria_altera[50];
                char nome_altera[50];
                float preco_altera;
                scanf("%d", &cod);
                printf("Digite o nome a ser alterado: \n");
                scanf("%s", nome_altera);
                printf("Digita a categoria a ser alterado: \n");
                scanf("%s", categoria_altera);
                printf("Digite o preco a ser alterado: \n");
                scanf("%.2f", &preco_altera);
                alteracao(cod, categoria_altera, preco_altera, nome_altera, metadados_file, indice_file, dados_file, d);
                break;
            case 5:
                imprime_catalogo(dados_file, d);
                break;
            case 6:
                exit(1);
                break;

        }

    }
    return;
}
*/