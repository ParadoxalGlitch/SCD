// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Prática 2. Introducción a los monitores en C++11.
//
// Archivo: prodcons_mu.cpp
//
// Ejemplo de un monitor en C++11 con semántica MU, para el problema
// del productor/consumidor, con productores y consumidores múltiples.
// Opcion FIFO
//
// Historial:
// Creado el 25 Oct de 2023. (adaptado de prodcons1_su.cpp)
// -----------------------------------------------------------------------------------



















#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   num_items = 16 ,   // número de items a producir/consumir
   num_prod = 4,
   num_cons = 4;
int
   siguiente_dato = 0, // siguiente valor a devolver en 'producir_dato'
   cantidad_producir = num_items / num_prod,
   cantidad_consumir = num_items / num_cons,
   producidos[num_prod];
   
constexpr int               
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for


mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items] = {0}, // contadores de verificación: producidos
   cont_cons[num_items] = {0}; // contadores de verificación: consumidos

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(int num_hebra)
{
   
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   const int valor_producido = (num_hebra*cantidad_producir) + producidos[num_hebra]; ;
   producidos[num_hebra]++;
   mtx.lock();
   cout << "hebra productora " << num_hebra << ", produce " << valor_producido << endl << flush ;
   mtx.unlock();
   cont_prod[valor_producido]++ ;
   return valor_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned valor_consumir )
{
   if ( num_items <= valor_consumir )
   {
      cout << " valor a consumir === " << valor_consumir << ", num_items == " << num_items << endl ;
      assert( valor_consumir < num_items );
   }
   cont_cons[valor_consumir] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   cout << "                  hebra consumidora, consume: " << valor_consumir << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class ProdConsSU1 : public HoareMonitor
{
 private:
 static const int           // constantes ('static' ya que no dependen de la instancia)
   num_celdas_total = 10;   //   núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//   buffer de tamaño fijo, con los datos
   primera_libre,         //   indice de celda de la próxima inserción ( == número de celdas ocupadas)
   primera_ocupada,
   usados;


 CondVar                    // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdConsSU1() ;             // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdConsSU1::ProdConsSU1(  )
{
   primera_libre = 0 ;
   primera_ocupada = 0;
   usados = 0;
   ocupadas      = newCondVar();
   libres        = newCondVar();

}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdConsSU1::leer(  )
{
   // esperar bloqueado hasta que 0 < primera_libre
   if ( usados == 0 )
      ocupadas.wait();

   //cout << "leer: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert( 0 < usados  );

   // hacer la operación de lectura, actualizando estado del monitor
   const int valor = buffer[primera_ocupada] ;
   usados--;
   primera_ocupada = (primera_ocupada + 1) % num_celdas_total ;
   
   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdConsSU1::escribir( int valor )
{
   // esperar bloqueado hasta que primera_libre < num_celdas_total
   if ( primera_libre == num_celdas_total )
      libres.wait();

   //cout << "escribir: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert( primera_libre < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   usados++;
   primera_libre = (primera_libre + 1) % num_celdas_total ;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdConsSU1> monitor, unsigned int num_hebra )
{
   for( unsigned i = 0; i < cantidad_producir; i++ )
   {
      int valor = producir_dato(num_hebra) ;
      monitor->escribir(valor);
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdConsSU1>  monitor, unsigned int num_hebra )
{
   for( unsigned i = 0 ; i < cantidad_consumir ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "---------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "---------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<ProdConsSU1> monitor = Create<ProdConsSU1>() ;

   thread hebra_productora[num_prod],
          hebra_consumidora[num_cons];

   for (int i=0; i<num_prod; i++){

      hebra_productora[i] = thread(funcion_hebra_productora, monitor, i);

   }

   for (int i=0; i<num_cons; i++){

      hebra_consumidora[i] = thread(funcion_hebra_consumidora, monitor, i);

   }


   for (int i=0; i<num_prod; i++){

      hebra_productora[i].join();

   }

   for (int i=0; i<num_cons; i++){

      hebra_consumidora[i].join();

   }

   test_contadores() ;
}
