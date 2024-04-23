#include <stdlib.h>
#include <stdio.h>
#include <utils/structures.h>
#include <utils/paquete.h>
#include <utils/setup.h>
#include <utils/socket.h>
#include <utils/environment_variables.h>
#include <commons/bitarray.h>



t_log* logger;
t_config* config;
uint32_t socket_memoria;
uint32_t socket_kernel;
char* tipo_interfaz;
int tiempo_unidad_de_trabajo;
int block_size;
int block_count;
char* path_base_dialfs;
int nombre_interfaz;
FILE* archivo_de_bloques;
int tam_archivo_de_bloques;
char* puntero_bitarray;
t_bitarray* bloques_libres;
FILE* archivo_bitmap;
FILE* archivo_metadata;
char* nombre_archivo;
op_code codigo_operacion;
uint32_t direccion_fisica;
int ip_kernel;
int puerto_memoria;
int ip_memoria;
int puerto_kernel;
char* nombre_archivo_bloques;
char* nombre_archivo_bitmap;
t_config* archivo_metadata_config;
int bloque_inicial_archivo_metadata;
int tamanio_archivo_metadata;
t_bitarray* bitarray_bloques;

void conectar_modulos();

struct 
{
    int bloque_inicial;
    int tamanio_archivo;
} t_bloque;
