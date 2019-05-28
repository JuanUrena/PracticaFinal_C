#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "token.h"
#include "read.h"
#include "rutes.h"

void subs_env(struct list *mylist){
	int i=0;
	struct cell *aux=mylist->first;
	
	while(i<mylist->number_element){
		aux->ins=env_variable(aux->ins);
		aux=aux->next;
		i++;
	}
}


int main(int argc, char *argv[])
{
	char *text;
//	char *text2;

	
	struct list *ins_list;
	struct list *arg_list;
	//struct list *list_equiality;
	struct cell *ins;
//	struct list *comand;
	struct comands *list_comand;
	struct comands *list_comand2;
	struct comands *aux;
	
	struct param_cmd *cmd_line;	
//	struct cell *aux;	

  
	do{
		list_comand=NULL;
		arg_list=(struct list *) malloc (sizeof(struct list));
		free(arg_list);
		int i=0;
	//La linea de comandos
		text=read_line();
	//	text2=strdup(text);
	//compruebo si es EOF
		if(text){
			if (!strlen(text)){
				free(text);
				printf("\n-----FIN-----");
	  			return 0;
			}
			
			cmd_line=get_in_out(text);
			free(text);
			//printf("\n%s\n%s\n%s\n",cmd_line->comand, cmd_line->in, cmd_line->out);
		//lista de las instrucciones 
			if (cmd_line->comand){
				ins_list=tokenizar(cmd_line->comand, "|");
				//print(ins_list);
			
			
			//obtengo para cada instruccion su lista de argumentos. 
				ins=ins_list->first;
				//printf("\nee:\n%d",ins_list->number_element);
				while(i<ins_list->number_element){
				
					arg_list=tokenizar(ins->ins, " ");
					if(!list_comand){
						//printf("SE AÑADE EL PRIMER ELEMENTO");
						list_comand=(struct comands *) malloc (sizeof(struct comands));
						list_comand->list=NULL;
						list_comand->next=NULL;
					
						list_comand->list=arg_list;
					}else{
						//printf("SE AÑADE OTRO ELEMENTO");
						list_comand2=list_comand;
						while (list_comand2){
							aux=list_comand2;
							list_comand2=list_comand2->next;
						}
						list_comand2=(struct comands *) malloc (sizeof(struct comands));
						list_comand2->next=NULL;
						list_comand2->list=arg_list;
						aux->next=list_comand2;
					}
				
					ins=ins->next;
				
					i++;
				}
				
			//	print(ins_list);
			//	ins=ins_list->first;
			//	while (ins!=NULL){
			//		aux=ins->next;
			//		free(ins);
			//		ins=aux;
			//	}
			
			//Vamos a comprobar que cada elemento en la lista si es una asignacion de variable
			
				 list_comand2=list_comand;
				 struct value_var *check_var;
				 while (list_comand2!=NULL){
				 	getFiles(list_comand2->list);//globbing
				 	//Como hacer el globbing y a la vez la expansion del comando, o quizas deba hacerlo luego sobre valor y value?¿?¿?¿

				 	check_var=check_var_value(list_comand2->list->first->ins);
					printf("AQUI%d", check_var->var); 	
				 	if (check_var->var){
				 		//hacer globbing aqui?¿?¿
				 		check_var->variable=env_variable(check_var->variable);
				 		check_var->value=env_variable(check_var->value);
				 		
				 		//list_equiality=(struct list *) malloc (sizeof(struct list));
				 		//add_element(list_equiality, check_var->variable);
				 		//add_element(list_equiality, check_var->variable);
				 		printf("\nAsignacion de Variable\n%s\n%s\n", check_var->variable,check_var->value);
				 		
				 	}else{
				 		printf("\nEjecución de comando\n");
				 		subs_env(list_comand2->list);
				 		getFiles(list_comand2->list);//globbing
				 		print(list_comand2->list);
				 		
				 	}
				 	list_comand2=list_comand2->next;
				 }
				
		
				//printf("\n3:\n%s",text2);
				//print_all(list_comand);
				free_all(list_comand);
				free_list(ins_list);
				free(cmd_line->comand);
				free(cmd_line->in);
				free(cmd_line->out);
				free(cmd_line);				
			}
		}

	}while(1);
  
	return 0;
};
