/*
 * NSGAII.c
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Helper.h"
#include "NSGAII.h"
#include "Calculations.h"


Rota *ROTA_CLONE;

void malloc_rota_clone(){
	/*Criando uma rota para c�pia e valida��o das rotas*/
	ROTA_CLONE = (Rota*) calloc(1, sizeof(Rota));
	ROTA_CLONE->list = calloc(MAX_SERVICES_MALLOC_ROUTE, sizeof(Service));
}

/*Adiciona o indiv�duo de rank k no front k de FRONTS
 * Atualiza o size de FRONTS caso o rank seja maior*/
void add_Individuo_front(Fronts * fronts, Individuo *p){
	Population *fronti = fronts->list[p->rank];
	if (fronts->size < p->rank + 1){
	  fronti->size = 0;
	  fronts->size++;
	}
	
	fronti->list[fronti->size] = p;
	fronti->size++;
}

/*Verifica se A domina B (melhor em pelo menos 1 obj)*/
bool dominates(Individuo *a, Individuo *b){
	bool smaller_found = false;
	for (int i = 0; i < QTD_OBJECTIVES; i++){
		if (a->objetivos[i] < b->objetivos[i])
			smaller_found = true;
		if (a->objetivos[i] > b->objetivos[i])
			return false;
	}
	return smaller_found;
}

void add_dominated(Individuo *a, Individuo *b){
	if (a->dominates_list == NULL){
		a->dominates_list = malloc(32* sizeof(Individuo*));
		a->dominates_list_capacity = 32;
	}

	if (a->dominates_list_capacity <= a->dominates_list_count){
		a->dominates_list_capacity += 32;
		Individuo **temp = realloc(a->dominates_list, a->dominates_list_capacity * sizeof(Individuo*));
		if (temp != NULL) a->dominates_list = temp;
	}
	a->dominates_list[a->dominates_list_count] = b;
	a->dominates_list_count++;

}

/*Ordena os indiv�duos segundo o crit�rio de n�o domina��o*/
void fast_nondominated_sort(Population *population, Fronts * fronts){
	/*====================Zerando o frontlist==================================*/
	for (int i = 0; i < fronts->size; i++){
		fronts->list[i]->size = 0;
	}
	fronts->size = 0;

	/*===================Zerando o dominated counts============================*/
	for (int i = 0; i < population->size; i++){
		population->list[i]->dominated_by_count = 0;
		population->list[i]->dominates_list_count = 0;
	}
	
	//Primeiro passo, computando as dominancias
	for (int i = 0; i < population->size; i++){
		Individuo *a = population->list[i];
		for (int j = 0; j < population->size; j++){
			if (i == j) continue;
			Individuo *b = population->list[j];
			if (dominates(a,b)){
				add_dominated(a, b);
			}
			else if (dominates(b,a)){
				a->dominated_by_count++;
			}
		}
		if (a->dominated_by_count == 0){
			a->rank = 0;
			add_Individuo_front(fronts, a);
		}
	}
	
	int index_front = 0;
	//Iterando enquanto existirem novos fronts
	while (index_front < fronts->size){
		Population * front_i = fronts->list[index_front];
		//Iterando sobre os elementos do front index_front
		for (int i = 0; i < front_i->size; i++){
			Individuo *p = front_i->list[i];

			for (int k = 0; k < p->dominates_list_count; k++){
				Individuo *indv_dominated = p->dominates_list[k];
				if (indv_dominated->dominated_by_count > 0){
					indv_dominated->dominated_by_count--;
				}
				if (indv_dominated->dominated_by_count == 0){
					indv_dominated->rank = index_front+1;
					add_Individuo_front(fronts, indv_dominated);
					indv_dominated->dominated_by_count = -1;//Evita que o mesmo idv seja re-add em outras iteracoes
					//count_reps++;
				}
			}
		}
		index_front++;
	}
}

/*Pra poder usar a fun��o qsort com N objetivos,
 * precisamos implementar os n algoritmos de compare*/
int compare0(const void *p, const void *q) {
    int ret;
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
    if (x->objetivos[0] == y->objetivos[0])
        ret = 0;
    else if (x->objetivos[0] < y->objetivos[0])
        ret = -1;
    else
        ret = 1;
    return ret;
}

int compare1(const void *p, const void *q) {
    int ret;
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
    if (x->objetivos[1] == y->objetivos[1])
        ret = 0;
    else if (x->objetivos[1] < y->objetivos[1])
        ret = -1;
    else
        ret = 1;
    return ret;
}

int compare2(const void *p, const void *q) {
    int ret;
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
    if (x->objetivos[2] == y->objetivos[2])
        ret = 0;
    else if (x->objetivos[2] < y->objetivos[2])
        ret = -1;
    else
        ret = 1;
    return ret;
}

int compare3(const void *p, const void *q) {
    int ret;
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
    if (x->objetivos[3] == y->objetivos[3])
        ret = 0;
    else if (x->objetivos[3] < y->objetivos[3])
        ret = -1;
    else
        ret = 1;
    return ret;
}

/*Ordena a popula��o de acordo com o objetivo 0, 1, 2, 3*/
void sort_by_objective(Population *pop, int obj){
	switch(obj){
		case 0:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare0 );
			break;
		case 1:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare1 );
			break;
		case 2:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare2 );
			break;
		case 3:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare3 );
			break;
	}
}


/*Deve ser chamado depois de determinar as fun��es objetivo*/
void crowding_distance_assignment(Population *pop){
	for (int i = 0; i < pop->size; i++){
		pop->list[i]->crowding_distance = 0;
	}
	for (int k = 0; k < QTD_OBJECTIVES; k++){

		sort_by_objective(pop, k);

		pop->list[0]->crowding_distance = FLT_MAX;
		pop->list[pop->size -1]->crowding_distance = FLT_MAX;

		float obj_min = pop->list[0]->objetivos[k];//valor min do obj k
		float obj_max = pop->list[pop->size -1]->objetivos[k];//valor max do obj k

		float diff = fmax(0.0001, obj_max - obj_min);

		for (int z = 1; z < pop->size -1; z++){
			float prox_obj = pop->list[z+1]->objetivos[k];
			float ant_obj = pop->list[z-1]->objetivos[k];

			if (pop->list[z]->crowding_distance != FLT_MAX)
				pop->list[z]->crowding_distance += (prox_obj - ant_obj) / diff;
		}

	}
}

/*Pra poder usar a fun��o qsort com N objetivos,
 * precisamos implementar os n algoritmos de compare*/
int compareByCrowdingDistanceMax(const void *p, const void *q) {
    int ret;
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
    if (x->crowding_distance == y->crowding_distance)
        ret = 0;
    else if (x->crowding_distance > y->crowding_distance)
        ret = -1;
    else
        ret = 1;
    return ret;
}



void sort_by_crowding_distance_assignment(Population *front){
	//crowding_distance_assignment(front); //jah eh feito antes
	qsort(front->list, front->size, sizeof(Individuo*), compareByCrowdingDistanceMax );
}

bool crowded_comparison_operator(Individuo *a, Individuo *b){
	return (a->rank < b->rank || (a->rank == b->rank && a->crowding_distance > b->crowding_distance));
}

/*
 *Atualiza os tempos de inser��o no ponto de inser��o at� o fim da rota.
 * */
void update_times(Rota *rota, int posicao_insercao){
	for (int i = posicao_insercao-1; i < rota->length-1; i++){
		Service *ant = &rota->list[i];
		Service *actual = &rota->list[i+1];

		actual->service_time = calculate_time_at(actual, ant);

		/*
		if (actual->is_source)
			actual->waiting_time = fmax(0, actual->r->pickup_earliest_time - actual->service_time);
		else
			actual->waiting_time = 0;
		*/
	}
}


/*
 * A0101B = tamanho 6
 * 			de 0 a 5
 * 	Inserir na posi�ao 0 n�o pode pq j� tem o motorista
 * 	Inserir na posi��o 1, empurra os demais pra frente
 * 	Inserir o destino � contado � partir da origem (offset)
 * 	offset = 0 n�o pode pq � o proprio origem, 1 pode e � o pr�ximo,
 * 	2 � o que pula um e insere.
 * */
bool insere_carona_rota(Rota *rota, Request *carona, int posicao_insercao, int offset){
	if (posicao_insercao <= 0 || offset <= 0) return false;

	clone_rota(rota, ROTA_CLONE);

	Service * anterior = &ROTA_CLONE->list[posicao_insercao-1];
	Service * proximo = &ROTA_CLONE->list[posicao_insercao];
	double pickup_result;
	double delivery_result;

	//if (!is_insercao_rota_valida_jt(anterior, proximo, carona, &pickup_result, &delivery_result))
		//return false;

	int ultimaPos = ROTA_CLONE->length-1;
	//Empurra todo mundo depois da posi��o de inser��o
	for (int i = ultimaPos; i >= posicao_insercao; i--){
		ROTA_CLONE->list[i+1].is_source = ROTA_CLONE->list[i].is_source;
		ROTA_CLONE->list[i+1].offset = ROTA_CLONE->list[i].offset;
		ROTA_CLONE->list[i+1].r = ROTA_CLONE->list[i].r;
		ROTA_CLONE->list[i+1].service_time = ROTA_CLONE->list[i].service_time;
		//ROTA_CLONE->list[i+1].waiting_time = ROTA_CLONE->list[i].waiting_time;
	}
	//Empurra todo mundo depois da posi��o do offset
	for (int i = ultimaPos+1; i >= posicao_insercao + offset; i--){
		ROTA_CLONE->list[i+1].is_source = ROTA_CLONE->list[i].is_source;
		ROTA_CLONE->list[i+1].offset = ROTA_CLONE->list[i].offset;
		ROTA_CLONE->list[i+1].r = ROTA_CLONE->list[i].r;
		ROTA_CLONE->list[i+1].service_time = ROTA_CLONE->list[i].service_time;
		//ROTA_CLONE->list[i+1].waiting_time = ROTA_CLONE->list[i].waiting_time;
	}

	//Insere o conte�do do novo carona
	ROTA_CLONE->list[posicao_insercao].r = carona;
	ROTA_CLONE->list[posicao_insercao].is_source = true;
	//ROTA_CLONE->list[posicao_insercao].service_time = pickup_result;
	//Insere o conte�do do destino do carona
	ROTA_CLONE->list[posicao_insercao+offset].r = carona;
	ROTA_CLONE->list[posicao_insercao+offset].is_source = false;
	//ROTA_CLONE->list[posicao_insercao+offset].service_time = delivery_result;

	ROTA_CLONE->length += 2;
	carona->matched = true;

	if (ROTA_CLONE->length == ROTA_CLONE->capacity - 4){
		ROTA_CLONE->capacity += MAX_SERVICES_MALLOC_ROUTE;
		ROTA_CLONE->list = realloc(ROTA_CLONE->list, ROTA_CLONE->capacity * sizeof(Service));
	}

	//Depois de inserir, deve verificar se o ponto anterior precisa ser atualizado
	//Se for, o push_forward faz o resto do trabalho de corrigir os tempos de pickup e delivery
	//ROTA_CLONE->list[i+1].service_time = ROTA_CLONE->list[i].service_time;

	update_times(ROTA_CLONE, posicao_insercao);

	bool rotaValida = is_rota_valida(ROTA_CLONE);

	if (rotaValida){
		clone_rota(ROTA_CLONE, rota);
		return true;
	}
	else{
		carona->matched = false;
		return false;
	}

//	if (!rotaValida){
//		desfaz_insercao_carona_rota(ROTA_CLONE, posicao_insercao, offset);
//		carona->matched = false;
//		update_times(ROTA_CLONE);
//		return false;
//	}
//	clone_rota(ROTA_CLONE, rota);
//	return true;
}

void desfaz_insercao_carona_rota(Rota *rota, int posicao_insercao, int offset){
	if (posicao_insercao <= 0 || offset <= 0) return;

	if (rota->length == 2)
		printf("rota vai ficar vazia");

	for (int i = posicao_insercao; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;

	for (int i = posicao_insercao+offset-1; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;
}

/*Remove a marca��o de matched dos riders*/
void clean_riders_matches(Graph *g){
	for (int i = g->drivers; i < g->total_requests; i++){
		g->request_list[i].matched = false;
	}
}

void evaluate_objective_functions_pop(Population* p, Graph *g){
	for (int i = 0; i < p->size; i++){//Pra cada um dos indiv�duos
		evaluate_objective_functions(p->list[i], g);
		//clean_riders_matches(g);
	}
}


void evaluate_objective_functions(Individuo *idv, Graph *g){
	double distance = 0;
	double vehicle_time = 0;
	double rider_time = 0;
	double riders_unmatched = g->riders;
	for (int m = 0; m < idv->size; m++){//pra cada rota
		Rota *rota = &idv->cromossomo[m];

		vehicle_time += tempo_gasto_rota(rota, 0, rota->length-1);
		distance += distancia_percorrida(rota);

		for (int i = 0; i < rota->length-1; i++){//Pra cada um dos sources services
			Service *service = &rota->list[i];
			if (service->r->driver || !service->is_source)//s� contabiliza os services source que n�o � o motorista
				continue;
			riders_unmatched--;
			//Repete o for at� encontrar o destino
			//Ainda n�o considera o campo OFFSET contido no typedef SERVICE
			for (int j = i+1; j < rota->length; j++){
				Service *destiny = &rota->list[j];
				if(destiny->is_source || service->r != destiny->r)
					continue;

				rider_time += tempo_gasto_rota(rota, i, j);
				break;
			}
		}
	}

	idv->objetivos[TOTAL_DISTANCE_VEHICLE_TRIP] = distance;
	idv->objetivos[TOTAL_TIME_VEHICLE_TRIPS] = vehicle_time;
	idv->objetivos[TOTAL_TIME_RIDER_TRIPS] = rider_time;
	idv->objetivos[RIDERS_UNMATCHED] = riders_unmatched;

}



/*Insere uma quantidade vari�vel de caronas na rota informada
 * Utilizado na gera��o da popula��o inicial, e na repara��o dos indiv�duos quebrados*/
void insere_carona_aleatoria_rota(Graph *g, Rota* rota){
	Request * request = &g->request_list[rota->id];

	int qtd_caronas_inserir = request->matchable_riders;
	if (qtd_caronas_inserir == 0) return;
	/*Configurando o index_array usado na aleatoriza��o
	 * da ordem de leitura dos caronas*/
	for (int l = 0; l < qtd_caronas_inserir; l++){
		index_array_caronas_inserir[l] = l;
	}

	//int qtd_caronas_inserir = VEHICLE_CAPACITY;
	shuffle(index_array_caronas_inserir, qtd_caronas_inserir);

	for (int z = 0; z < qtd_caronas_inserir; z++){
		Request * carona = request->matchable_riders_list[index_array_caronas_inserir[z]];
		int posicao_inicial = 1 + (rand () % (rota->length-1));
		int offset = 1;//TODO, variar o offset
		if (!carona->matched)
			insere_carona_rota(rota, carona, posicao_inicial, offset);
	}
}


/*Pega os melhores N indiv�duos do frontList e joga na popula��o pai.
 * Os restantes v�o pra popula��o filho.
 * Remove da lista de pais e filhos as listas de domina��o
 * "esvazia" o frontsList
 * */
void select_parents_by_rank(Fronts *frontsList, Population *parents, Population *offsprings, Graph *g){
	int lastPosition = 0;
	parents->size = 0;
	offsprings->size = 0;


	/*Para cada um dos fronts, enquanto a qtd de elementos dele couber inteiramente em parents, vai adicionando
	 * Caso contr�rio para. pois da� pra frente, s� algums desses indiv�duos ir�o para o parent
	 * o restante desse front em lastPosition e dos pr�ximos fronts v�o pro offsprings*/
	for (int i = 0; i < frontsList->size; i++){
		Population * front_i = frontsList->list[i];
		lastPosition = i;
		crowding_distance_assignment(front_i);
		if (parents->max_capacity - parents->size >= front_i->size){
			for (int j = 0; j < front_i->size; j++){
				parents->list[parents->size++] = front_i->list[j];
			}
		}
		else{
			break;
		}
	}

	int restantes_adicionar = parents->max_capacity - parents->size;//Qtd que tem que adicionar aos pais

	//Se restantes_adicionar > 0 ent�o o front atual n�o comporta todos os elementos de parent
	if (restantes_adicionar > 0){
		sort_by_crowding_distance_assignment(frontsList->list[lastPosition]);//ordena
		for (int k = 0; k < restantes_adicionar; k++){
			parents->list[parents->size++] = frontsList->list[lastPosition]->list[k];//Adiciona o restante aos pais
		}
		//Inserindo no filho o restante desses indiv�duos que n�o couberam nos pais
		for (int k = restantes_adicionar; k < frontsList->list[lastPosition]->size; k++){
			offsprings->list[offsprings->size++] = frontsList->list[lastPosition]->list[k];
		}
		lastPosition++;
	}


	/*Adicionar todos os restantes de bigpopulation aos filhos*/
	while (lastPosition < frontsList->size){
		for (int k = 0; k < frontsList->list[lastPosition]->size; k++){
			offsprings->list[offsprings->size] = frontsList->list[lastPosition]->list[k];
			offsprings->size++;
		}
		lastPosition++;
	}
}

/*Copia o conte�do das duas popula��es na terceira.
 * � uma c�pia simples, onde assume-se que os indiv�duos est�o na heap
 * "esvazia" p1 e p2*/
void merge(Population *p1, Population *p2, Population *big_population){
	big_population->size = 0;//Zera o bigpopulation

	for (int i = 0; i < p1->size + p2->size; i++){
		if (i < p1->size){
			big_population->list[i] = p1->list[i];
		}
		else{
			big_population->list[i] = p2->list[i - p1->size];
		}
	}
	big_population->size = p1->size + p2->size;
}

/*sele��o por torneio, k = 2*/
Individuo * tournamentSelection(Population * parents){
	Individuo * best = NULL;
	for (int i = 0; i < 2; i++){
		int pos = rand() % parents->size;
		Individuo * outro = parents->list[pos];
		if (best == NULL || crowded_comparison_operator(outro, best))
			best = outro;
	}
	return best;
}

void crossover(Individuo * parent1, Individuo *parent2, Individuo *offspring1, Individuo *offspring2, Graph *g, float crossoverProbability){
	int rotaSize = g->drivers;
	offspring1->size = rotaSize;
	offspring2->size = rotaSize;

	int crossoverPoint = 1 + (rand() % (rotaSize-1));
	float accept = (float)rand() / RAND_MAX;

	if (accept < crossoverProbability){
		copy_rota(parent2, offspring1, 0, crossoverPoint);
		copy_rota(parent1, offspring1, crossoverPoint, rotaSize);
		copy_rota(parent1, offspring2, 0, crossoverPoint);
		copy_rota(parent2, offspring2, crossoverPoint, rotaSize);
	}
	else{
		copy_rota(parent1, offspring1, 0, rotaSize);
		copy_rota(parent2, offspring2, 0, rotaSize);
	}
}

/*Remove todas as caronas que quebram a valida��o
 * Tenta inserir novas
 * Utiliza graph pra saber quem j� fez match.
 * */
void repair(Individuo *offspring, Graph *g, int position){

	for (int i = 0; i < offspring->size; i++){//Pra cada rota do idv
		Rota *rota = &offspring->cromossomo[i];

		for (int j = 0; j < rota->length; j++){//pra cada um dos services SOURCES na rota
			//Se � matched ent�o algum SOURCE anterior j� usou esse request
			//Ent�o deve desfazer a rota de j at� o offset
			if ((rota->list[j].is_source && rota->list[j].r->matched)){
				int offset = 1;
				for (int k = j+1; k < rota->length; k++){//encontrando o offset
					if (rota->list[j].r == rota->list[k].r && !rota->list[k].is_source)
						break;
					offset++;
				}
				desfaz_insercao_carona_rota(rota, j, offset);
			}
			else if (!rota->list[j].r->driver){
				rota->list[j].r->matched = true;
			}
		}
		insere_carona_aleatoria_rota(g, rota);
	}
}

/** 1a muta��o: remover o carona de uma rota e inserir em um motorista onde s� cabe um carona*/
void mutation(Individuo *ind, Graph *g, float mutationProbability){
	float accept = (float)rand() / RAND_MAX;

	if (accept < mutationProbability){


	}


}



/*Gera uma popula��o de filhos, usando sele��o, crossover e muta��o*/
void crossover_and_mutation(Population *parents, Population *offspring,  Graph *g, float crossoverProbability, float mutationProbability){
	int index_array[g->riders];
	for (int l = 0; l < g->riders; l++){
		index_array[l] = l;
	}

	offspring->size = 0;//Tamanho = 0, mas considera todos j� alocados
	int i = 0;
	while (offspring->size < parents->size){

		Individuo *parent1 = tournamentSelection(parents);
		Individuo *parent2 = tournamentSelection(parents);

		Individuo *offspring1 = offspring->list[i++];
		Individuo *offspring2 = offspring->list[i];

		crossover(parent1, parent2, offspring1, offspring2, g, crossoverProbability);

		clean_riders_matches(g);
		shuffle(index_array, g->riders);
		repair(offspring1, g, 1);

		clean_riders_matches(g);
		shuffle(index_array, g->riders);
		repair(offspring2, g, 2);

		mutation(offspring1, g, mutationProbability);
		mutation(offspring2, g, mutationProbability);
		offspring->size += 2;
	}
}

