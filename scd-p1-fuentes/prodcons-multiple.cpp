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
   num_prod = 4,
   num_cons = 4;
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}; // contadores de verificación: para cada dato, número de veces que se ha consumido.


   // Vector de datos
   int cantidad_producir = num_items / num_prod;
   int cantidad_consumir = num_items / num_cons;
   
   int items[tam_vec];
   int producidos[num_prod] = {0};
   int primera_ocupada = 0;
   int primera_libre = 0;



   Semaphore ocupadas = 0;
   Semaphore libres = tam_vec;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int i)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = i*cantidad_producir + producidos[i];
   producidos[i]++;
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

void  funcion_hebra_productora(unsigned int i)
{
   for(int j = 0 ; j < cantidad_producir ; j++ )
   {
      int dato = producir_dato(i) ;

      sem_wait(libres);

      items[primera_libre++] = dato;
      primera_libre %= tam_vec;
      
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(unsigned int i)
{
   for( unsigned i = 0 ; i < cantidad_consumir ; i++ )
   {
      sem_wait(ocupadas);
      int dato ;

      dato = items[primera_ocupada++];
      primera_ocupada %= tam_vec;
      
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

      hebra_productora[i] = thread(funcion_hebra_productora, i);

   }

   for (int i=0; i<num_cons; i++){

      hebra_consumidora[i] = thread(funcion_hebra_consumidora, i);

   }


   for (int i=0; i<num_prod; i++){

      hebra_productora[i].join();

   }

   for (int i=0; i<num_cons; i++){

      hebra_consumidora[i].join();

   }


   test_contadores();
}
