#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 
const int num_lectores = 3;
const int num_escritores = 3;


// *****************************************************************************
// clase para monitor Estanco, version FIFO, semántica SC, multiples prod/cons

class Lec_Esc : public HoareMonitor
{
 private:
 bool
  escrib;
int
  n_lec;

 CondVar                // colas condicion:
   lectura,             //  cola donde esperan los lectores
   escritura;           //  cola donde esperan los escritores

 public:                   // constructor y métodos públicos
   Lec_Esc();              // constructor
   void ini_lectura(); // extrae del mostrador un ingrediente
   void fin_lectura(); // coloca un ingrediente en el mostrador
   void ini_escritura(); // espera a que el mostrador esté vacio
   void fin_escritura(); // espera a que el mostrador esté vacio
} ;
// -----------------------------------------------------------------------------


Lec_Esc::Lec_Esc()
{
    escrib = false;
    n_lec = 0;
    lectura = newCondVar();
    escritura = newCondVar();

}

void Lec_Esc::ini_lectura()
{
    if (escrib)
        lectura.wait();
    n_lec++;
    lectura.signal();
}

void Lec_Esc::fin_lectura()
{

n_lec = n_lec - 1;

if (n_lec == 0)
    escritura.signal();
}

void Lec_Esc::ini_escritura()
{
    if (n_lec > 0 || escrib)
        escritura.wait();
    escrib = true;
}

void Lec_Esc::fin_escritura()
{
    escrib = false;
    if (!lectura.empty())
        lectura.signal();
    else
        escritura.signal();
}



//----------------------------------------------------------------------
// función que ejecuta la hebra del lector

void funcion_hebra_lector(MRef<Lec_Esc> monitor, int n_lector)
{

    while (true){

        monitor->ini_lectura();
      
        // calcular milisegundos aleatorios de duración de la acción de leer)
        chrono::milliseconds duracion_lec( aleatorio<10,100>() );

        cout << "Lector " << n_lector << "ha empezado a leer" << endl;

        // espera bloqueada un tiempo igual a ''duracion_lec' milisegundos
        this_thread::sleep_for( duracion_lec );

        cout << "Lector " << n_lector << "ha acabado de leer" << endl;

        monitor->fin_lectura();

        //"Resto del codigo"

        chrono::milliseconds duracion_resto( aleatorio<100,1000>() );

        this_thread::sleep_for( duracion_resto );


   }

}


//----------------------------------------------------------------------
// función que ejecuta la hebra del escritor
void funcion_hebra_escritor(MRef<Lec_Esc> monitor, int n_escritor)
{

   while( true )
   {
        monitor->ini_escritura();

        // calcular milisegundos aleatorios de duración de la acción de escribir)
        chrono::milliseconds duracion_esc( aleatorio<10,100>() );

        cout << "Escritor " << n_escritor << "ha empezado a escribir" << endl;

        // espera bloqueada un tiempo igual a ''duracion_esc' milisegundos
        this_thread::sleep_for( duracion_esc );

        cout << "Escritor " << n_escritor << "ha acabado de escribir" << endl;

        monitor->fin_escritura();

        chrono::milliseconds duracion_resto( aleatorio<10,100>() );

        this_thread::sleep_for( duracion_resto );
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
   MRef<Lec_Esc> monitor = Create<Lec_Esc>() ;


   thread hebra_lector[num_lectores],
          hebra_escritor[num_escritores];

   for (int i=0; i<num_lectores; i++){

      hebra_lector[i] = thread(funcion_hebra_lector, monitor, i);

   }

   for (int j=0; j<num_escritores; j++){

      hebra_escritor[j] = thread(funcion_hebra_escritor, monitor, j);

   }


   for (int i=0; i<num_lectores; i++){

        hebra_lector[i].join();

   }

   for (int j=0; j<num_escritores; j++){

        hebra_escritor[j].join();

   }

return 0;

}