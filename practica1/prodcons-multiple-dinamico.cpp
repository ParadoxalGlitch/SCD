#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"


using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 40 ,   // número de items
	tam_vec   = 10 ,  // tamaño del buffer
   num_prod = 8,
   num_cons = 5;
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}; // contadores de verificación: para cada dato, número de veces que se ha consumido.


   // Vector de datos
   int items[tam_vec];
   int siguiente_dato= 0;
   int primera_ocupada = 0;
   int primera_libre = 0;

   int producidos = num_items;
   int consumidos = 0;


   Semaphore ocupadas = 0;
   Semaphore libres = tam_vec;
   Semaphore prod_leyendo = 1;
   Semaphore cons_leyendo = 1;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato;
   siguiente_dato++;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;

   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora()
{

    while(producidos > 0){
      producidos--;
      int dato = producir_dato();

      sem_wait(libres);

      sem_wait(prod_leyendo);

      items[primera_libre] = dato;
      primera_libre = (primera_libre + 1) % tam_vec;

      sem_signal(prod_leyendo);
      
      sem_signal(ocupadas);
    }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora()
{

   while(consumidos < num_items)
   {
      consumidos++;
      int dato;
      
      sem_wait(ocupadas);
      sem_wait(cons_leyendo);

      dato = items[primera_ocupada];
      primera_ocupada = (primera_ocupada + 1)%tam_vec;

      sem_signal(cons_leyendo);
      
      sem_signal(libres);

      consumir_dato( dato );
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO o FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;


   thread hebra_productora[num_prod],
          hebra_consumidora[num_cons];

   for (int i=0; i<num_prod; i++){

      hebra_productora[i] = thread(funcion_hebra_productora);

   }

   for (int i=0; i<num_cons; i++){

      hebra_consumidora[i] = thread(funcion_hebra_consumidora);

   }


   for (int i=0; i<num_prod; i++){

      hebra_productora[i].join();

   }

   for (int i=0; i<num_cons; i++){

      hebra_consumidora[i].join();

   }


   test_contadores();
}
