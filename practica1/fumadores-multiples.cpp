#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 8 ;

Semaphore mostr_vacio = 1;
Semaphore ingr_disp[3] = {0,0,0};

string ingredientes[num_fumadores] = {"Cerillas", "Tabaco", "Papel"};

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,2>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << ingredientes[num_ingrediente] << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{

   int i;

   while (true){

      i = producir_ingrediente();
      sem_wait(mostr_vacio);
      cout << "           Puesto ingrediente: " << ingredientes[i] << endl;
      sem_signal(ingr_disp[i]);

   }

}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{

   while( true )
   {

      sem_wait(ingr_disp[num_fumador%3]);
      cout << "           Retirado ingrediente: " << ingredientes[num_fumador%3] << endl;
      sem_signal(mostr_vacio);
      fumar(num_fumador);

   
   }
}

//----------------------------------------------------------------------

int main()
{


   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;


   thread hebra_fumador[num_fumadores],
          hebra_estanquero;

   for (int i=0; i<num_fumadores; i++){

      hebra_fumador[i] = thread(funcion_hebra_fumador, i);

   }

   hebra_estanquero = thread(funcion_hebra_estanquero);


   for (int i=0; i<num_fumadores; i++){

      hebra_fumador[i].join();

   }

      hebra_estanquero.join();

}
