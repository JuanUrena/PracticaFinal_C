#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <err.h>

//Paquetes propios
#include "list.h"
#include "input.h"
#include "propsys.h"


struct 	conex{
	int input;
	int output;
};


//Elimina los espacios de un string
void remove_spaces(char* source)
{
	char* i = source;
	char* j = source;
	while(*j != 0){
		*i = *j++;
	if(*i != ' ')
		i++;
	}
	*i = 0;
}


//sustituimos en la lista las posibles variables de entorno
void subs_env(struct list *mylist){
	int i=0;
	struct cell *aux=mylist->first;
	
	while(i<mylist->number_element){
		aux->ins=env_variable(aux->ins);
		aux=aux->next;
		i++;
	}
}


//Cambia en el string el char actual por el nuevo
void replace_char(char* string, char actual, char new){
	if (string){
		for(int i=0; i<strlen(string); i++){
			if (string[i]==actual){
				string[i]=new;
			}
		}
	}
}


//Funcion para generar el array desde mi lista
void generate_array(glob_t glob, char *arr[glob.gl_pathc+1]){
	int i=0;
	for(i=0;i < glob.gl_pathc; i++ ){
		arr[i]=glob.gl_pathv[i];
	}
	arr[i]=NULL; 
}


void move_glob(glob_t glob){
	int i;
	glob.gl_pathc--;
	for(i=0;i < glob.gl_pathc; i++ ){
		glob.gl_pathv[i]=glob.gl_pathv[i+1];
	}
	glob.gl_pathv[i]=NULL;
}

// Function to execute builtin commands 
int ownCmdHandler(glob_t glob) 
{ 
	int NoOfOwnCmds = 3;
	int i;
	char *aux;
	int switchOwnArg = 0; 
	char* ListOfOwnCmds[NoOfOwnCmds]; 
	int result=1;
	ListOfOwnCmds[0] = "cd";
	ListOfOwnCmds[1] = "ifok";
	ListOfOwnCmds[2] = "ifnot"; 

	for (i = 0; i < NoOfOwnCmds; i++) { 
		if (!strcmp(glob.gl_pathv[0], ListOfOwnCmds[i])) { 
			switchOwnArg = i + 1; 
			break; 
		} 
	} 
	switch (switchOwnArg) { 
	case 1: 
		
		if (glob.gl_pathc>1){
			aux=glob.gl_pathv[1];
		}else{
			aux=getenv("HOME");
		}
		if (!aux){
			perror("Env error");
			exit(EXIT_FAILURE);
		}
		if(!(result=chdir(aux))){
			return result;
		}else{
			perror("System error");
			exit(EXIT_FAILURE);
		}
		//extesion 2
	case 2:
		aux=getenv("result");
		if (!aux){
			perror("Env error");
			exit(EXIT_FAILURE);
		}
		if (!strcmp(aux, "0")){
			move_glob(glob);
			return 1;
		}else{
			return 0;
		}
	case 3: 
		aux=getenv("result");
		if (!aux){
			perror("Env error");
			exit(EXIT_FAILURE);
		}
		if (strcmp(aux, "0")){
			move_glob(glob);
			return 1;
		}else{
			return 0;
		}
	default: 
		break; 
	} 
	return 1; 
} 


int waitchilds(){
	int childs;
	int lastchild=0;
	int result=0;
	int  status;
	while ((childs=wait(&status))>0){	
		if(status == -1) {
			perror("Error during wait()");
			exit(EXIT_FAILURE);
		}
		if (childs>lastchild){
			result=status;
			lastchild=childs;
		}
	}

	printf("\n-----FIN-----");
	return(result);
}


void son_code(glob_t glob){
	int i=0;
	char *route;
	
	if (glob.gl_pathc){
		char *arr[glob.gl_pathc+1];
		route=get_route(glob.gl_pathv[0]);	
		if (route){	
			for(i=0;i < glob.gl_pathc; i++ ){
				arr[i]=glob.gl_pathv[i];
			}
			arr[i]=NULL; 
			execv(route,arr);			
		}else{
			exit (0);
		}
		globfree(&glob);
	}
}


char* prepare_value(char *word){
	char *value=NULL;
	glob_t globbuf;
	int i;
	
	if (word){
		glob(word, GLOB_NOCHECK, NULL, &globbuf);
		for(i = 0; i < globbuf.gl_pathc; i++ ){
			if (value){
				value=(char *) realloc(value, strlen(value)+strlen(globbuf.gl_pathv[i])+2);
				if (!value){
					perror("Memory error");
					exit(EXIT_FAILURE);
				}
				strcat(value, ":");
			}else{
				value=(char *) calloc(1,strlen(globbuf.gl_pathv[i])+1);
				if (!value){
					perror("Memory error");
					exit(EXIT_FAILURE);
				}
			}
			strcat(value, globbuf.gl_pathv[i]);
		}
		globfree(&globbuf);		
	}else{
		value=strdup("\0");
		if (!value){
			perror("Memory error");
			exit(EXIT_FAILURE);
		}
	}
	return value;
}


void env_var_code(struct value_var *check_var){
	char *var_form;
	
	check_var->variable=env_variable(check_var->variable);
	check_var->value=env_variable(check_var->value);
	 						 
	var_form=prepare_value(check_var->value);
	
	if (setenv(check_var->variable, var_form, 1)<0){
		perror("Env error");
		exit(EXIT_FAILURE);
	}		 		
	free(check_var->variable);
	free(var_form);
}


struct conex* modelate_pipe(int i, int total,char *file, int output){
	int in=0;
	int out=0;
	int fd[2];
	
	struct conex *result=(struct conex*) malloc (sizeof(struct conex));
	if (!result){
		perror("Memory error");
		exit(EXIT_FAILURE);
	}
	if(i==total){
		if (file){
			remove_spaces(file);							
			out=open(file, O_WRONLY|O_CREAT|O_TRUNC, 
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
		} else{
			out=dup(output);
			if (out<0){
				perror("File error");
				exit(EXIT_FAILURE);
			}
		}
		
	}else{
		if (pipe(fd)==-1){
			perror("Pipe error");
			exit(EXIT_FAILURE);
		}
		out=fd[1];
		in=fd[0];
	}
	result->input=in;
	result->output=out;
	return result;
}


struct param_cmd* process_line(char *cmd_line){
	struct param_cmd *param_cmd_line;
	int exit_child=0;
	if(cmd_line){
		if (!strlen(cmd_line)){
			free(cmd_line);
			exit_child=waitchilds();
			exit(exit_child);
		}
		replace_char(cmd_line, '\t', ' ');
		param_cmd_line=param_line(cmd_line);
		free(cmd_line);
		return param_cmd_line;
	}else{
		return NULL;
	}
}

int process_input(char* file, int wait, int input){
	int in;
	if (file){
	//estamos quitando espacios sin mas, si estan en medio dejarlos?¿
		remove_spaces(file);
		in=open(file, O_RDONLY);
	}else if(!wait){
		in =open("/dev/null",O_RDONLY); 
	} else{
		in =dup(input);
	}
	if (in<0){
		perror("File error");
		exit(EXIT_FAILURE);
	}
	return in;
}

int wait_cmd_child(int num_child, int last_child)
{
	int status;
	int result=1;
	for(int x=0;x<num_child;x++){ 
		int pid=waitpid(-1, &status, 0);
		if WIFEXITED(status){
			if (pid==last_child){
				result=status;
			}
   		} 
	}
	return result;
}


void change_result(int result){
	char result_c[sizeof("%d")];
	int aux=sprintf(result_c, "%d",result);
	if (aux<0){
		perror("Memory error");
		exit(EXIT_FAILURE);
	}	
	if (setenv("result", result_c, 1)<0){
		perror("Env error");
		exit(EXIT_FAILURE);
	}
}

   
int main(int argc, char *argv[])
{
	char *text;
	char *here_line;
	char *end;
	char *buff;
	char *own_env_var="result";
	struct conex *pipe_conex;
	struct list *ins_list;
	//struct cell *ins;
	struct comands *list_comand;
	struct comands *list_comand2;
	//struct comands *aux;
	struct value_var *check_var;
	struct param_cmd *cmd_line;	
	
	int child;
	int num=0;
	int input=0;
	int output=0;
	int in;
	int out;
	int n_cmd;
	int fd[2];
	int chr_written;
	
	int exit_cmd;
  
	do{
		//Supongo que al comienzo venimos de un exito
		setenv(own_env_var, "0", 1);
		list_comand=NULL;
		text=read_line();
		cmd_line=process_line(text);
		if (cmd_line && cmd_line->comand){
			ins_list=tokenizar(cmd_line->comand, "|");
			n_cmd=ins_list->number_element;
			list_comand=cmdlist2cmdmatrix(ins_list);
			free_list(ins_list);
			list_comand2=list_comand;
			
			input=dup(0);
			output=dup(1);
			if (input<0 || output<0){
				perror("Memory error");
				exit(EXIT_FAILURE);
			} 
			//ext
			num=0;
			if (cmd_line->here && !num){
				if (pipe(fd)==-1){
					perror("Pipe error");
					exit(EXIT_FAILURE);
				}
				out=fd[1];
				in=fd[0];
			}else{
				in =process_input(cmd_line->in, cmd_line->wait, input);
			}
				
			
			while (list_comand2!=NULL){	
				check_var=check_var_value(list_comand2->list->first->ins);
				if (check_var->var){
					env_var_code(check_var);
					n_cmd--;
				}else{
					subs_env(list_comand2->list);
					glob_t glob=expand_arg(list_comand2->list);
					if(ownCmdHandler(glob)){
						if (dup2(in,0)<0){
							perror("Redirection error");
							exit(EXIT_FAILURE);
						}
						close (in);

						pipe_conex=modelate_pipe(num, n_cmd-1,cmd_line->out,output);	
						in=pipe_conex->input;
						if (dup2(pipe_conex->output,1)<0){
							perror("Redirection error");
							exit(EXIT_FAILURE);
						}
						close(pipe_conex->output);
						free(pipe_conex);
				 		
						child=fork();
						switch(child){
						case 0:
							if (cmd_line->here &&!num){
								close(fd[1]);
							}
							son_code(glob);
							free_all(list_comand);
							free(cmd_line->comand);
							free(cmd_line->in);
							free(cmd_line->out);
							free(cmd_line);
							break;
						case -1:
							perror("Fork error");
							exit(EXIT_FAILURE);
							break;
						}
						num++;
					}else{
						n_cmd--;
					}
					globfree(&glob);
				}
				list_comand2=list_comand2->next;
			}
			if (dup2(input, 0)<0 ||dup2(output,1)<0){
				perror("Redirection error");
				exit(EXIT_FAILURE);
			}
			if (cmd_line->here){
				close(fd[0]);
				end=strdup("}");
				
				if (!end){
					perror("File error");
					exit(EXIT_FAILURE);
				}
				
				buff=read_line();
				here_line=(char *)calloc(1,1);
				if (!here_line){
					perror("Memory error");
					exit(EXIT_FAILURE);
				}
				while(strcmp(buff, end)){
					here_line=realloc(here_line, strlen(here_line)+2+strlen(buff));
					if (!here_line){
						perror("Memory error");
						exit(EXIT_FAILURE);
					} 
					strcat(here_line, buff);
					strcat(here_line, "\n");
					free(buff);
					buff=read_line();
				}
				chr_written=write(out, here_line, strlen(here_line));
				if (chr_written!=strlen(here_line)){
					perror("Writing error");
					exit(EXIT_FAILURE);
				}
				free(here_line);
				close(out);
				free(end);
				if (dup2(input, 0)<0 ||dup2(output,1)<0){
					perror("Redirection error");
					exit(EXIT_FAILURE);
				}
				close(input);
				close(output);
			}
			if (cmd_line->wait){
				exit_cmd=wait_cmd_child(num, child);
				change_result(exit_cmd);
			}
			free_all(list_comand);
			free(cmd_line->comand);
			free(cmd_line->in);
			free(cmd_line->out);
			free(cmd_line);				
		}
		
	}while(1);
	return exit_cmd;
}
