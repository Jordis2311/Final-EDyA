#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

typedef struct _SNodo {
  char* palabra;
  struct _SNodo *sig;
}SNodo;

typedef SNodo *SList;

typedef struct _TablaHash{
  SList* elementos; // Array de listas para asi resolver colisiones con listas enlazadas
  int capacidad;
  int ocupados;
}_TablaHash;

typedef _TablaHash *TablaHash;

/**
 * Funcion de hash para strings propuesta por Kernighan & Ritchie en "The C
 * Programming Language (Second Ed.)".
 */
unsigned KRHash(char *s) {
  unsigned hashval;
  for (hashval = 0; *s != '\0'; ++s) {
    hashval = *s + 31 * hashval;
  }
  return hashval;
}

SList slist_crear() { return NULL; }

TablaHash tablahash_crear(int capacidad) {

  // Pedimos memoria para la estructura principal y las casillas.
  TablaHash tabla = malloc(sizeof(struct _TablaHash));
  tabla->elementos = malloc(sizeof(SList) * capacidad);
  assert(tabla->elementos != NULL);
  tabla->ocupados = 0;
  tabla->capacidad = capacidad;

  // Inicializamos las casillas con datos nulos.
  for (unsigned idx = 0; idx < capacidad; ++idx) {
    tabla->elementos[idx] = slist_crear();
  }

  return tabla;
}

void borrarlista(SList lista){
  while(lista->sig != NULL){
    SList eliminador = lista;
    lista = lista->sig;
    free(eliminador->palabra);
    free(eliminador);
  }
  free(lista->palabra);
  free(lista);
}

void eliminar_tabla(TablaHash tabla){
  for(int idx = 0;idx < tabla->capacidad;idx++){
    if(tabla->elementos[idx] != NULL)
      borrarlista(tabla->elementos[idx]);
  }
  free(tabla->elementos);
  free(tabla);
}

void tablahash_insertar(TablaHash tabla, char *dato) {

  // Calculamos la posicion del dato dado, de acuerdo a la funcion hash.
  unsigned idx = KRHash(dato) % tabla->capacidad;

  // Insertar el dato si la casilla estaba libre.
  if (tabla->elementos[idx] == NULL) {
    tabla->ocupados++;
    tabla->elementos[idx] = malloc(sizeof(SNodo));
    tabla->elementos[idx]->palabra = strdup(dato);
    tabla->elementos[idx]->sig = NULL;
    return;
  }
  // Si hay colision colocar el nuevo elemento en el siguiente nodo de la lista
  else if (strcmp(tabla->elementos[idx]->palabra,dato ) != 0) {
    SList temp = tabla->elementos[idx];
    for(; temp->sig!=NULL;temp = temp->sig);
    temp->sig = malloc(sizeof(SNodo));
    temp->sig->palabra = strdup(dato);
    temp->sig->sig = NULL;
    tabla->ocupados++;
    return;
  }
  // Ignorar si colisiona con el mismo elemento
  else {
    return;
  }
}

TablaHash Rehashear_tabla(TablaHash tabla){
  int actual = tabla->capacidad;
  int nueva = abs(actual*2);
  TablaHash nueva_tabla = tablahash_crear(nueva);
  for (int idx = 0; idx < actual; ++idx) {
    SList temp = tabla->elementos[idx];
    while(temp != NULL){
      tablahash_insertar(nueva_tabla,temp->palabra);
      temp = temp->sig;
    }
  }
  nueva_tabla->capacidad = nueva;
  eliminar_tabla(tabla);
  return nueva_tabla;
}

void mostrarlista(SList lista){
  SList temp = lista;
  for(; temp !=NULL;temp = temp->sig){
    printf("%s\n",temp->palabra);
  }
}

void mostrar_tablahash(TablaHash tabla){
  for(int idx = 0; idx < tabla->capacidad; idx++){
    if(tabla->elementos[idx] != NULL){
      printf("%i ",idx);
      mostrarlista(tabla->elementos[idx]);
    }
  }

}

TablaHash crear_diccionario(){
  FILE *dicc = fopen("es1.txt","r+");

  TablaHash diccionario = tablahash_crear(1024);

  int MAX = 255;
  char buffer[MAX];
  while(!feof(dicc)){
    //Si la tabla se esta llenando duplicaremos su espacio
    if((diccionario->ocupados / diccionario->capacidad) > 0.75){
      diccionario = Rehashear_tabla(diccionario);
    }
    fscanf(dicc,"%s",buffer);
    tablahash_insertar(diccionario,buffer);
  }
  return diccionario;
}

int en_el_diccionario(TablaHash tabla,char* palabra){
  int idx = KRHash(palabra) % tabla->capacidad;

  int bandera = 0;
  SList temp = tabla->elementos[idx];
  while((temp != NULL) && (bandera == 0)){
    if(strcmp(temp->palabra,palabra) == 0)
      bandera = 1;
    else 
      temp = temp->sig;
  }
  return bandera;
}

void insertar_caracter(char* palabra,char caracter,int largo,int posicion){
  for(int i = largo;i >= posicion;i--){
    palabra[i+1] = palabra[i];
  }
  palabra[posicion] = caracter;
}

void eliminar_caracter(char* palabra,int largo,int posicion){
  for(int i = posicion;i<largo;i++){
    palabra[i] = palabra[i+1];
  }
}

int no_en_lista(SList lista,char* palabra){
  int bandera = 1;
  for(SList temp = lista;temp != NULL;temp = temp->sig){
    if(strcmp(temp->palabra,palabra) == 0)
      bandera = 0;
  }
  return bandera;
}

SList slist_agregar(SList lista,char* palabra){
  SList temp = slist_crear();
  temp = malloc(sizeof(SNodo));
  temp->palabra = strdup(palabra);
  temp->sig = lista;
  
  return temp;
}


void buscar_a_distancia(TablaHash diccionario, char* palabra,int* sugeridas,int distancia,SList lista){

  int sug = *sugeridas;
  int largo = strlen(palabra);
  char mod[largo + 3],c;
  strcpy(mod,palabra);

  for(int i = 0;(i <= largo)  && (sug < 5);i++){
    //metodo 1: intercambiar caracteres
    if(i < largo - 1){
      c = mod[i];
      mod[i] = mod[i + 1];
      mod[i + 1] = c;

      if(distancia == 0){
        if(en_el_diccionario(diccionario,mod) && no_en_lista(lista,mod)){
          lista = slist_agregar(lista,mod);
          sug++;
        }
      }
      else
        buscar_a_distancia(diccionario,mod,&sug,distancia - 1,lista);

      mod[i+1] = mod[i];
      mod[i] = c;
    }
    
    //metodo 2: agregar caracteres
    for(int b = 97;(b < 123) && (sug < 5);b++){
      c = (char) b;
      insertar_caracter(mod,c,largo,i);

      if(distancia == 0){
        if(en_el_diccionario(diccionario,mod) && no_en_lista(lista,mod)){
          lista = slist_agregar(lista,mod);
          sug++;
        }
      }
      else
        buscar_a_distancia(diccionario,mod,&sug,distancia - 1,lista);

      eliminar_caracter(mod,largo+1,i);
    }

    //Metodo 3: eliminar caracter
    if(i < largo){
      c = mod[i];
      eliminar_caracter(mod,largo,i);

      if(distancia == 0){
        if(en_el_diccionario(diccionario,mod) && no_en_lista(lista,mod)){
          lista = slist_agregar(lista,mod);
          sug++;
        }
      }
      else
        buscar_a_distancia(diccionario,mod,&sug,distancia - 1,lista);

      insertar_caracter(mod,c,largo-1,i);

    //Metodo 4: cambiar caracter por otro
      for(int b = 97;(b < 123) && (sug < 5);b++){
        c = mod[i];
        mod[i] = (char) b;
        
        if(distancia == 0){
          if(en_el_diccionario(diccionario,mod) && no_en_lista(lista,mod)){
            lista = slist_agregar(lista,mod);
            sug++;
          }
        }
        else
          buscar_a_distancia(diccionario,mod,&sug,distancia - 1,lista);

      mod[i] = c;
      }
    }
  }

  mostrarlista(lista);

  *sugeridas = sug;
}

int isEmpty(SList lista){
  return lista == NULL;
}

void sugerir_palabras(TablaHash diccionario, char* palabra){
  int sugeridas = 0;

  SList sugerencias = slist_crear();

  printf("Distancia 1: ");
  buscar_a_distancia(diccionario,palabra,&sugeridas,0,sugerencias);

  mostrarlista(sugerencias);
  puts("");

  printf("Distancia 2: ");
  buscar_a_distancia(diccionario,palabra,&sugeridas,1,sugerencias);
  puts("");


}

void protocolo_error(TablaHash errores,TablaHash diccionario,char* palabra,int linea){
  if(en_el_diccionario(errores,palabra) == 0){
    //Si la tabla se esta llenando duplicaremos su espacio
    if((1.5*errores->ocupados) > errores->capacidad){
      errores = Rehashear_tabla(errores);
    }
    tablahash_insertar(errores,palabra);
    printf("Linea: %i | %s | No se ha encontrado en el diccionario\n",linea,palabra);
    printf("Quizas quizo decir: ");
    sugerir_palabras(diccionario,palabra);
  }
}

void corrector(TablaHash diccionario){
  FILE  *texto = fopen("texto.txt","r+");
  TablaHash errores = tablahash_crear(10);

  int MAX = 255, c = 0, linea = 1;
  char buffer[MAX];
  char caracter;

  while(!feof(texto)){
    fscanf(texto,"%c",&caracter);
    switch (caracter){
      case ' ':
        buffer[c] = '\0';
        if(c > 1)
          if(en_el_diccionario(diccionario,buffer) == 0)
            protocolo_error(errores,diccionario,buffer,linea);
        c = 0;
        break;
      case '\n':
        //
        if (buffer[c - 1] == '\r') buffer[c - 1] = '\0';
        else buffer[c] = '\0';
        if(c > 1)
          if(en_el_diccionario(diccionario,buffer) == 0)
            protocolo_error(errores,diccionario,buffer,linea);
        c = 0;
        break;
      case ',': case '.': case ';': case ':': case '?': case '!':
        buffer[c] = '\0';
        if(c > 1)
          if(en_el_diccionario(diccionario,buffer) == 0)
            protocolo_error(errores,diccionario,buffer,linea);
        c = 0;
        break;
      default:
        buffer[c] = tolower(caracter);
        c++;
        break;
    }
  }
  buffer[c-1] = '\0';
  if(c > 1)
    if(en_el_diccionario(diccionario,buffer) == 0)
      protocolo_error(errores,diccionario,buffer,linea);

  //mostrar_tablahash(errores);
  eliminar_tabla(errores);
}

int main(){

  TablaHash diccionario = crear_diccionario();
  //mostrar_tablahash(diccionario);

  corrector(diccionario);


  eliminar_tabla(diccionario);
  return 0;
}
