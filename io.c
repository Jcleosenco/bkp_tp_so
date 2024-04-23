#include "io.h"




int main(int argc, char* argv[]) {
    initialize_setup(IO_CONFIG_PATH, "io", &logger, &config);
	conectar_modulos(); 

    //inicializar_config();
    //inicializar_estructura_fs();

    tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ"); 

    iniciar_interfaz();

    destruir_cerrar_estructuras();

    while(1) {};
}

void conectar_modulos() {
    socket_memoria = start_client_module("MEMORIA", IP_CONFIG_PATH);
	log_info(logger, "Conexion con servidor MEMORIA en socket %d", socket_memoria);

    socket_kernel = start_client_module("KERNEL", IP_CONFIG_PATH);
	log_info(logger, "Conexion con servidor KERNEL en socket %d", socket_kernel);
}

void iniciar_interfaz() {

    if(tipo_interfaz == "GENERICA") { 
        manejar_interfaz_generica();
    }
    if(tipo_interfaz == "STDIN")
        manejar_interfaz_STDIN();
    if(tipo_interfaz == "STDOUT")
        manejar_interfaz_STDOUT();
    if(tipo_interfaz == "DialFS")
        manejar_interfaz_DialFS();
}

// void inicializar_config() {
//     tiempo_unidad_de_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
//     block_size = config_get_int_value(config, "BLOCK_SIZE");
//     block_count = config_get_int_value(config, "BLOCK_COUNT");
//     path_base_dialfs = config_get_int_value(config, "PATH_BASE_DIALFS");
// }

void manejar_interfaz_generica() {
    tiempo_unidad_de_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_int_value(config, "IP_KERNEL");
    puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");

    // van a esperar una cantidad de unidades de trabajo, cuyo valor va a venir dado en la petición desde el Kernel.
    int valor_sleep;

    sleep(tiempo_unidad_de_trabajo * valor_sleep);
}

void manejar_interfaz_STDIN(uint32_t direccion_fisica) {
    ip_kernel = config_get_int_value(config, "IP_KERNEL");
    puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
    ip_memoria = config_get_int_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");

    // espera que se ingrese texto por teclado (ver si implementar semaforo)

    char buffer[100]; // Buffer para almacenar el texto ingresado
    printf("Ingresa un texto: ");
    
    // Lee una línea de texto desde la entrada estándar (teclado)
    fgets(buffer, sizeof(buffer), stdin);
    printf("Ingresaste: %s", buffer);

    // se guarda texto en memoria en la direccion fisica recibida por kernel
}

void manejar_interfaz_STDOUT() {
    tiempo_unidad_de_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_int_value(config, "IP_KERNEL");
    puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
    ip_memoria = config_get_int_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");

    // conectarse a memoria, leer dir fisica y mostrar resultado
    tiempo_unidad_de_trabajo--;
}

void manejar_interfaz_DialFS() {
    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    path_base_dialfs = config_get_int_value(config, "PATH_BASE_DIALFS");
    ip_memoria = config_get_int_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    op_code codigo_operacion;

    inicializar_estructura_fs();
    tiempo_unidad_de_trabajo--;

    switch(codigo_operacion){
        case IO_FS_CREATE:
            crear_archivo();
            break;
        case IO_FS_DELATE:
            break;
        case IO_FS_TRUNCATE:
            int tamanio_nuevo;
            truncar_archivo(tamanio_nuevo);
            break;
        case IO_FS_WRITE:
            break;
        case IO_FS_READ:
            break;
    }
}

void inicializar_estructura_fs() {
    tam_archivo_de_bloques = block_size * block_count;
    // nombre_archivo se envia como parte de la instruccion (ver de obtenerlo)
    inicializar_archivo_de_bloques();
    inicializar_bitmap();
    inicializar_archivo_metadata(nombre_archivo);
}

void inicializar_archivo_de_bloques(){
    // concateno archivo de bloques al path de archvios dialfs
    nombre_archivo_bloques = "bloques.dat";
    size_t long_path = strlen(path_base_dialfs) + strlen(nombre_archivo_bloques);
    char* path = malloc(long_path);
    snprintf(path, "%s/%s", path_base_dialfs, nombre_archivo_bloques);

    FILE* archivo_de_bloques = fopen(path, "r+");
    if (archivo_de_bloques == NULL) {
        log_error(logger, "error al abrir el archivo bloques.dat");
        exit(EXIT_FAILURE);
    }

}

void inicializar_bitmap(){
    // concateno archivo bitmap al path de archvios dialfs
    nombre_archivo_bitmap = "bitmap.dat";
    size_t long_path = strlen(path_base_dialfs) + strlen(nombre_archivo_bitmap);
    char* path = malloc(long_path);
    snprintf(path, "%s/%s", path_base_dialfs, nombre_archivo_bitmap);

    FILE* archivo_bitmap = fopen(path, "r+");
    if (archivo_bitmap == NULL) {
        log_error(logger, "error al abrir el archivo bitmap.dat");
        exit(EXIT_FAILURE);
    }
    
    // leo el bitarray contenido en el archivo
    fseek(archivo_bitmap, 0, SEEK_END);
    size_t tamanio_archivo = ftell(archivo_bitmap);
    fseek(archivo_bitmap, 0, SEEK_SET);
    
    t_bitarray* bitarray_bloques = bitarray_create_with_mode(NULL, tamanio_archivo, LSB_FIRST);
    if (bitarray_bloques == NULL) {
        log_error(logger, "error al crear el bitarray");
        fclose(archivo_bitmap);
        free(path);
        exit(EXIT_FAILURE);
    }

    fread(bitarray_bloques->bitarray, sizeof(char), tamanio_archivo, archivo_bitmap);
}

void inicializar_archivo_metadata(nombre_archivo){
    size_t long_path = strlen(path_base_dialfs) + strlen(nombre_archivo);
    char* path = malloc(long_path);
    snprintf(path, "%s/%s", path_base_dialfs, nombre_archivo);

    archivo_metadata = fopen(path, "r+");
    if (archivo_metadata == NULL) {
        log_error(logger, "error al abrir el archivo ", nombre_archivo);
        exit(EXIT_FAILURE);
    }

    archivo_metadata_config = config_create(path); 

    bloque_inicial_archivo_metadata = config_get_int_value(archivo_metadata_config, "BLOQUE_INICIAL");
    tamanio_archivo_metadata = config_get_int_value(archivo_metadata_config, "TAMANIO_ARCHIVO");
}


void crear_archivo() {
    int cant_bloques_a_asignar = redondear_para_arriba((double)tamanio_archivo_metadata / block_size);
    // cuando se crea el archivo su bloque inicial es -1 para establecer que no tiene bloque asignado, asi lo interpreta la funcion asignar bloques contiguos
    bloque_inicial_archivo_metadata = -1;

    asignar_bloques_contiguos_bitarray(cant_bloques_a_asignar, tamanio_archivo_metadata, 0);

    int bloque_inicial_contiguo = -1;
    int bloques_contiguos_encontrados = 0;

    log_info(logger, "PID: ", "Crear archivo: ", nombre_archivo);



    // for(int i = 0; i < block_size; i++) {
    //     if(bitarray_test_bit(bitarray_bloques, i) == 0) {
    //         if(bloque_inicial_contiguo == -1) {
    //             bloque_inicial_contiguo = i;
    //             bloques_contiguos_encontrados++;
    //         } 
    //     }
    //     else {
    //         bloque_inicial_contiguo = -1;
    //         bloques_contiguos_encontrados = 0;
    //     }

    //     if(bloques_contiguos_encontrados >= cant_bloques_a_asignar)
    //         break;
    // }

    // if(bloque_inicial_contiguo != -1) {
    //     int cant_bloques_a_asignar_aux = cant_bloques_a_asignar;
    //     int bloque_inicial_contiguo_aux = bloque_inicial_contiguo;

    //     // marco bloques como ocupados en bitarray (bloques que asigno al archivo)
    //     while(cant_bloques_a_asignar_aux > 0) {
    //         bitarray_set_bit(bitarray_bloques, bloque_inicial_contiguo_aux);
    //         bloque_inicial_contiguo_aux++;

    //         cant_bloques_a_asignar_aux--;
    //     }

    //     // marco en el archivo su nuevo bloque inicial
    //     config_set_value(archivo_metadata_config, "BLOQUE_INICIAL", bloque_inicial_contiguo);
    //     bloque_inicial_archivo_metadata = bloque_inicial_contiguo;

    // }   else {
    //     log_info(logger, "no se encontro espacio contiguo para asignar bloques al archivo");
    // }
}


void truncar_archivo(int tamanio_nuevo) {

    log_info(logger, "PID: ", "nombre_archivo: ", nombre_archivo, " tamanio: ", tamanio_nuevo);

    if(tamanio_archivo_metadata - tamanio_nuevo > 0)
        agrandar_tamanio_archivo(tamanio_nuevo, tamanio_archivo_metadata);
    else {
        reducir_tamanio_archivo(tamanio_nuevo, tamanio_archivo_metadata);
    }

    tamanio_archivo_metadata = tamanio_nuevo;
}

void agrandar_tamanio_archivo(int tamanio_nuevo, int tamanio_viejo) {
    // ver el caso en que no hayan bloques contiguos luego de los ya asignados

    int cant_bloques_a_agregar = redondear_para_arriba((tamanio_nuevo - tamanio_archivo_metadata) / block_size);
    asignar_bloques_contiguos_bitarray(cant_bloques_a_agregar, tamanio_viejo, tamanio_nuevo);
}

void reducir_tamanio_archivo(int tamanio_nuevo, int tamanio_viejo) {
    int cant_bloques_a_sacar = redondear_para_arriba((tamanio_archivo_metadata - tamanio_nuevo) / block_size);

    sacar_bloques_bitmap(cant_bloques_a_sacar, tamanio_viejo);
}

// retorna el bloque inicial 
void asignar_bloques_contiguos_bitarray(int cant_bloques_a_asignar, int tamanio_viejo, int tamanio_nuevo) {
    int bloque_inicial_contiguo = -1;
    int bloques_contiguos_encontrados = 0;

    for(int i = 0; i < block_size; i++) {
        if(bitarray_test_bit(bitarray_bloques, i) == 0) {
            if(bloque_inicial_contiguo == -1) {
                bloque_inicial_contiguo = i;
                bloques_contiguos_encontrados++;
            } 
        }
        else {
            bloque_inicial_contiguo = -1;
            bloques_contiguos_encontrados = 0;
        }

        if(bloques_contiguos_encontrados >= cant_bloques_a_asignar)
            break;
    }

    if(bloque_inicial_contiguo != -1) {
        int cant_bloques_a_asignar_aux = cant_bloques_a_asignar;
        int bloque_inicial_contiguo_aux = bloque_inicial_contiguo;

        // marco bloques como ocupados en bitarray (bloques que asigno al archivo)
        while(cant_bloques_a_asignar_aux > 0) {
            bitarray_set_bit(bitarray_bloques, bloque_inicial_contiguo_aux);
            bloque_inicial_contiguo_aux++;

            cant_bloques_a_asignar_aux--;
        }

        // marco en el archivo su nuevo bloque inicial
        if(bloque_inicial_archivo_metadata <= -1) {
            config_set_value(archivo_metadata_config, "BLOQUE_INICIAL", bloque_inicial_contiguo);
            bloque_inicial_archivo_metadata = bloque_inicial_contiguo;
        }

    }   else {
        log_info(logger, "no se encontro espacio contiguo para asignar bloques al archivo");
        compactar_fs(cant_bloques_a_asignar, tamanio_viejo, tamanio_nuevo);
    }

}


void sacar_bloques_bitmap(int cant_bloques_a_sacar, int tamanio_viejo) {
    int posicion_ultimo_bloque = bloque_inicial_archivo_metadata + redondear_para_arriba(tamanio_viejo / block_size);

    for(int i = posicion_ultimo_bloque; cant_bloques_a_sacar > 0; cant_bloques_a_sacar--) {
        bitarray_clean_bit(bitarray_bloques, i);
    }
}


void compactar_fs(int cant_bloques_a_asignar, int tamanio_viejo, int tamanio_nuevo) {
        // Crear una matriz para representar el estado de cada bloque
    // Block bloques[NUM_BLOCKS];

    // // Leer el estado actual de cada bloque del bitarray y actualizar la matriz
    // for (int i = 0; i < NUM_BLOCKS; i++) {
    //     bloques[i].ocupado = bitarray_test_bit(bitarray_bloques, i);
    // }

    // // Iterar sobre la matriz de bloques para encontrar bloques libres y ocupados
    // int indice_destino = 0;
    // for (int i = 0; i < NUM_BLOCKS; i++) {
    //     if (bloques[i].ocupado) {
    //         // Si el bloque está ocupado, mover sus datos al bloque de destino
    //         if (i != indice_destino) {
    //             // Calcular la posición de inicio y el tamaño del bloque a mover
    //             long inicio_origen = i * BLOCK_SIZE;
    //             long inicio_destino = indice_destino * BLOCK_SIZE;
    //             fseek(archivo_bloques, inicio_origen, SEEK_SET);
    //             char buffer[BLOCK_SIZE];
    //             fread(buffer, sizeof(char), BLOCK_SIZE, archivo_bloques);
    //             fseek(archivo_bloques, inicio_destino, SEEK_SET);
    //             fwrite(buffer, sizeof(char), BLOCK_SIZE, archivo_bloques);

    //             // Marcar el bloque original como libre y el destino como ocupado
    //             bloques[i].ocupado = false;
    //             bloques[indice_destino].ocupado = true;
    //         }
    //         indice_destino++;
    //     }
    // }

    // // Actualizar el bitarray con el nuevo estado de los bloques
    // for (int i = 0; i < NUM_BLOCKS; i++) {
    //     if (bloques[i].ocupado) {
    //         bitarray_set_bit(bitarray_bloques, i);
    //     } else {
    //         bitarray_clean_bit(bitarray_bloques, i);
    //     }
    // }

    if(bloque_inicial_archivo_metadata < 0) {

    }  else {
        // ya tiene bloques asignados
        int cant_bloques_asignados = redondear_para_arriba(tamanio_viejo / block_size);
        int cant_bloques_asignados_aux = cant_bloques_asignados;
        int i = bloque_inicial_archivo_metadata;
        while(cant_bloques_asignados_aux > 0){
            bitarray_clean_bit(bitarray_bloques, i);
            cant_bloques_asignados_aux--;
            i++;
        }

        if(tamanio_nuevo > 0) {
            int cant_bloques_nuevos_a_asignar = redondear_para_arriba(cant_bloques_asignados + tamanio_nuevo / block_size);
            int dim = contar_cant_bloques_libres();
            int posiciones_bloques_libres[dim];
            int j = 0;

            // obtengo posiciones de los bloques_libres
            for(int i = 0; i < block_size; i++) {
                if(bitarray_test_bit(bitarray_bloques, i) == 0) {
                    posiciones_bloques_libres[j] = i;
                    j++;
                }
            }

            //

        }
    }
    
}


int contar_cant_bloques_libres() {
    int cant_bloques_libres = 0;
    for(int i = 0; i < block_size; i++) {
        if(bitarray_test_bit(bitarray_bloques, i) == 0)
            cant_bloques_libres++;
    }

    return cant_bloques_libres;
}


int redondear_para_arriba(double resultado) {
    double parte_decimal = modf(resultado, &resultado);
    int parte_entera = resultado;

    if(parte_decimal > 0)
        return parte_entera ++;
    else return parte_entera; 
}

void destruir_cerrar_estructuras() {
    fclose(archivo_de_bloques);
    fclose(archivo_bitmap);
    fclose(nombre_archivo);
    fclose(archivo_bitmap);
    free(path_base_dialfs);
    bitarray_destroy(bitarray_bloques);
}