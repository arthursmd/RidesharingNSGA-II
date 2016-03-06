/*
 ============================================================================
 Name        : RidesharingNSGAII-Clean.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Helper.h"
#include "NSGAII.h"
#include "Calculations.h"

/*Parametros: nome do arquivo
 *
 * Inicia com 3 popula��es
 * Pais - Indiv�duos alocados
 * Filhos -Indiv�duos alocados
 * Big_population - indiv�duos N�O alocados*/
int main(int argc,  char** argv){

	/*Setup======================================*/
	if (argc < 4) {
		printf("Argumentos insuficientes\n");
		return 0;
	}
	srand (time(NULL));
	//Parametros (vari�veis)
	int POPULATION_SIZE;
	int ITERATIONS;
	int PRINT_ALL_GENERATIONS = 0;
	float crossoverProbability = 0.75;
	float mutationProbability = 0.1;
	char *filename = argv[1];

	sscanf(argv[2], "%d", &POPULATION_SIZE);
	sscanf(argv[3], "%d", &ITERATIONS);
	if (argc >= 5)
		sscanf(argv[4], "%f", &crossoverProbability);
	if (argc >= 6)
			sscanf(argv[5], "%f", &mutationProbability);
	if (argc >= 7)
		sscanf(argv[6], "%d", &PRINT_ALL_GENERATIONS);
	Graph * g = (Graph*)parse_file(filename);
	if (g == NULL) return 0;

	/*============================================*/

	/*Calculando os caronas que s�o combin�veis para cada motorista*/
	Service pickupMotorista, deliveryMotorista;

	for (int i = 0; i < g->drivers; i++){
		Request * motoristaGrafo = &g->request_list[i];
		pickupMotorista.r = motoristaGrafo;
		deliveryMotorista.r = motoristaGrafo;
		for (int j = g->drivers; j < g->total_requests; j++){
			Request * carona = &g->request_list[j];
			double pickup, delivery;
			if ( is_insercao_rota_valida_jt(&pickupMotorista, &deliveryMotorista, carona, &pickup, &delivery)) {
				motoristaGrafo->matchable_riders_list[motoristaGrafo->matchable_riders++] = carona;
			}
		}
	}


	/*Imprimindo quantos caronas cada motorista consegue fazer match*/
	int qtd = 0;
	printf("quantos matches cada motorista consegue\n");
	for (int i = 0; i < g->drivers; i++){
		if (g->request_list[i].matchable_riders > 0)
			qtd++;
		printf("%d: ",g->request_list[i].matchable_riders);
		for (int j = 0; j < g->request_list[i].matchable_riders; j++){
			printf("%d ", g->request_list[i].matchable_riders_list[j]->req_no);
		}
		printf("\n");
	}

	printf("qtd m�nima que deveria conseguir: %d\n", qtd);

	/*=====================In�cio do NSGA-II============================================*/

	
	Population *big_population = (Population*) new_empty_population(POPULATION_SIZE*2);
	Fronts *frontsList = new_front_list(POPULATION_SIZE * 2);
	
	Population * parents = generate_random_population(POPULATION_SIZE, g, true);
	Population * children = generate_random_population(POPULATION_SIZE, g, false);
	evaluate_objective_functions_pop(parents, g);

	int i = 0;
	while(i < ITERATIONS){
		printf("Iteracao %d...\n", i);
		evaluate_objective_functions_pop(children, g);
		merge(parents, children, big_population);
		fast_nondominated_sort(big_population, frontsList);

		//Aloca os melhores entre os pais e filhos (que foram parar em frontsList) e joga em pais
		//O restante ir� para os filhos, que de qualquer forma ser� sobreescrito pelo crossover.
		select_parents_by_rank(frontsList, parents, children, g);
		crossover_and_mutation(parents, children, g, crossoverProbability);
		if (PRINT_ALL_GENERATIONS)
			print(children);
		i++;
	}
	
	//Ao sair do loop, verificamos uma ultima vez o melhor gerado entre os pais e filhos
	evaluate_objective_functions_pop(parents, g);
	evaluate_objective_functions_pop(children, g);
	merge(parents, children, big_population);
	fast_nondominated_sort(big_population, frontsList);

	evaluate_objective_functions_pop(frontsList->list[0], g);
	printf("Imprimindo o ultimo front obtido:\n");
	sort_by_objective(frontsList->list[0], RIDERS_UNMATCHED);
	print(frontsList->list[0]);

	dealoc_full_population(parents);
	dealoc_full_population(children);
	dealoc_empty_population(big_population);
	//dealoc_fronts(frontsList); :(
	//dealoc_graph(g);
	return EXIT_SUCCESS;
}

