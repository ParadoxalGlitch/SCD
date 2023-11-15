#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 
const int num_fumadores = 7;
const int num_estanqueros = 4;
const string ingredientes[3] = {"Cerillas", "Tabaco", "Papel"};


// *****************************************************************************
// clase para monitor Estanco, version FIFO, semántica SC, multiples prod/cons

class Estanco : public HoareMonitor
{
 private:
 string
    mostrador;

 CondVar                    // colas condicion:
   espera_ingrediente,      //  cola donde espera los fumadores
   espera_recogida;   //  cola donde espera el estanquero

 public:                    // constructor y métodos públicos
   Estanco() ;              // constructor
   void obtenerIngrediente(int ingrediente); // extrae del mostrador un ingrediente
   void ponerIngrediente(int ingrediente); // coloca un ingrediente en el mostrador
   void esperarRecogidaIngrediente(); // espera a que el mostrador esté vacio
} ;
// -----------------------------------------------------------------------------


Estanco::Estanco()
{
    mostrador = "Vacio";
    espera_ingrediente = newCondVar();
    espera_recogida = newCondVar();

}

void Estanco::obtenerIngrediente(int ingrediente)
{

    while (mostrador == "Vacio")
        espera_ingrediente.wait();

    while (mostrador != ingredientes[ingrediente]){

        espera_ingrediente.signal();
        espera_ingrediente.wait();
    }


    mostrador = "Vacio";
    cout << "           Retirado ingrediente: " << ingredientes[ingrediente] << endl;
    espera_recogida.signal();

}

void Estanco::ponerIngrediente(int ingrediente)
{

    mostrador = ingredientes[ingrediente];
    cout << "           Puesto ingrediente: " << ingredientes[ingrediente] << endl;
    espera_ingrediente.signal();

}

void Estanco::esperarRecogidaIngrediente()
{

    if (mostrador != "Vacio"){
        espera_recogida.wait();
    }

}




//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente(int num_estanquero)
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero " << num_estanquero << ": empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,2>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero " << num_estanquero << ": termina de producir ingrediente " << ingredientes[num_ingrediente] << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(MRef<Estanco> monitor, int num_estanquero)
{

   int i;

   while (true){

      i = producir_ingrediente(num_estanquero);
      monitor->esperarRecogidaIngrediente();
      monitor->ponerIngrediente(i);
      

   }

}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<10,100>() );

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
void  funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador )
{

   while( true )
   {

      monitor->obtenerIngrediente(num_fumador%3);
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

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Estanco> monitor = Create<Estanco>() ;


   thread hebra_fumador[num_fumadores],
          hebra_estanquero[num_estanqueros];

   for (int i=0; i<num_fumadores; i++){

      hebra_fumador[i] = thread(funcion_hebra_fumador, monitor, i);

   }


   for (int j=0; j<num_estanqueros; j++){

      hebra_estanquero[j] = thread(funcion_hebra_estanquero, monitor, j);

   }


   for (int i=0; i<num_fumadores; i++){

      hebra_fumador[i].join();

   }


   for (int j=0; j<num_estanqueros; j++){

      hebra_estanquero[j].join();

   }


}