#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
#include <vector>


// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	static int par = 0;
	Action accion = actIDLE;
	estado copia;

	if (sensores.nivel != 4){
		if (!hayPlan){
			actual.fila = sensores.posF;
			actual.columna = sensores.posC;
			actual.orientacion = sensores.sentido;
			destino.fila = sensores.destinoF;
			destino.columna= sensores.destinoC;
			hayPlan = pathFinding(sensores.nivel, actual, destino, plan);
		}

		Action sigAction;
		if (hayPlan and plan.size()>0){
			sigAction = plan.front();
			plan.erase(plan.begin());
		}
		else{}

		return sigAction;
	}
	else{
		actual.fila = sensores.posF;
		actual.columna = sensores.posC;
		actual.orientacion = sensores.sentido;
		destino.fila = sensores.destinoF;
		destino.columna= sensores.destinoC;

		tiempo--;
		actualizar_mapa(sensores);

		if (sensores.bateria <= 100){
			par = -1;
		}

		int pasos = 7;
		if (par % pasos == 0 and par <= mapaResultado[1].size()*10)
			hayPlan = pathFinding(sensores.nivel, actual, destino, plan);
		++par;

		
			

		copia = actual;

		if(actual.fila == destino.fila and actual.columna == destino.columna)
			hayPlan = false;

		if (mapaResultado[actual.fila][actual.columna] == 'D')
			zapatillas = true;
		else if (mapaResultado[actual.fila][actual.columna] == 'K')
			bikini = true;


		Action sigAction;

	

		if (hayPlan and plan.size()>0){
			if (HayObstaculoDelante(copia) and plan.front() == actFORWARD){
				hayPlan = pathFinding (sensores.nivel, actual, destino, plan);
			}

			if (sensores.superficie[2] == 'a' and plan.front() == actFORWARD){
				sigAction = actIDLE;
			}
			else if (mapaResultado[actual.fila][actual.columna] == 'X' and tiempo > tiempo_mapa/3 and (sensores.bateria < tiempo_mapa/2)){
				sigAction = actIDLE;
			}
		
			else{
				sigAction = plan.front();
				plan.erase(plan.begin());
			}
			
		}
		else{
			hayPlan = pathFinding(sensores.nivel, actual, destino, plan);
		}

		return sigAction;
	}
  
  }


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen, destino, plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
						return pathFinding_CostoUniforme(origen, destino, plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
						return pathFinding_AEstrella(origen, destino, plan);
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}



struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

struct comparenodo{
	bool operator()(const nodo& nodo1, const nodo& nodo2) const{
		bool resultado = false;

		estado est1 = nodo1.st;
		estado est2 = nodo2.st;

		if (nodo1.coste > nodo2.coste)
      return true;
    else{
      if(nodo1.coste == nodo2.coste)
        ComparaEstados()(nodo1.st,nodo2.st);
      else
        return false;
    }

		return resultado;
	}
};

struct comparenodoheu{
	bool operator()(const nodo &nodo1, const nodo &nodo2) const{
		bool resultado = false;

		estado est1 = nodo1.st, est2 = nodo2.st;
		if (nodo1.heuristica > nodo2.heuristica)
					return true;
		else{
			if (nodo1.heuristica == nodo2.heuristica)
				ComparaEstados()(est1, est2);
			else
				return false;
		}
		return resultado;
	}
};




// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de generados
	stack<nodo> pila;											// Lista de pila

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}


// Implementación de la búsqueda en anchura.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de generados
	queue<nodo> pila;											// Lista de pila

  nodo current;
	current.st = origen;
	current.secuencia.empty();


	pila.push(current);

  while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}


// Implementación de la búsqueda en costo uniforme.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de generados
	priority_queue<nodo, vector<nodo>, comparenodo> pila; // Nodos por explorar	


	zapatillas = false;
	bikini = false;

 	nodo current;
	current.st = origen;
	current.secuencia.empty();
	current.coste = 0;
	current.zapatillas = false;
	current.bikini = false;

	pila.push(current);

  	while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		actualizarBikiniZapatillas(current);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		hijoTurnR.coste += calcular_costo(current.st, current.zapatillas, current.bikini);
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		hijoTurnL.coste += calcular_costo(current.st, current.zapatillas, current.bikini);
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			hijoForward.coste += calcular_costo(current.st, current.zapatillas, current.bikini);
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}


		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}

	return false;
}

// Distancia de Manhattan
int distManhattan(estado est1, estado est2){
	return abs(est1.fila-est2.fila) + abs(est1.columna-est2.columna);
}

bool ComportamientoJugador::pathFinding_AEstrella(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la listas
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de generados
	priority_queue<nodo, vector<nodo>, comparenodoheu> pila; // Nodos por explorar


	int regulador = mapaResultado.size()/100;

 	nodo current;
	current.st = origen;
	current.secuencia.empty();
	current.coste = 0;
	current.zapatillas = false;
	current.bikini = false;

	pila.push(current);

  	while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		if (bikini)
			current.bikini = true;
		if (zapatillas)
			current.zapatillas = true;

		actualizarBikiniZapatillas(current);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		hijoTurnR.coste += calcular_costo(current.st, current.zapatillas, current.bikini);
		hijoTurnR.heuristica =hijoTurnR.coste + regulador*distManhattan(hijoTurnR.st, destino);
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		hijoTurnL.coste += calcular_costo(current.st, current.zapatillas, current.bikini);
		hijoTurnL.heuristica = hijoTurnL.coste  + regulador*distManhattan(hijoTurnL.st, destino);
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			hijoForward.coste += calcular_costo(current.st, current.zapatillas, current.bikini);
			hijoForward.heuristica = hijoForward.coste  + regulador*distManhattan(hijoForward.st, destino);
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}


		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}

	return false;
}


// Función para actualizar el mapa.
void ComportamientoJugador::actualizar_mapa(Sensores sensores){
	int x    = actual.fila,
    y    = actual.columna,
    cont = 0;

  if(mapaResultado[x][y] == '?') 
    mapaResultado[x][y] = sensores.terreno[0];
  
  switch (sensores.sentido){
    case este:
      cont = -1; 
      for(int i=1; i<4; ++i){  
        if(mapaResultado[x + cont][y + 1] == '?')
          mapaResultado[x + cont][y + 1] = sensores.terreno[i];
        cont++;
      }

      cont = -2; 
      for(int i=4; i<9; ++i){  
        if(mapaResultado[x + cont][y + 2] == '?')
          mapaResultado[x + cont][y + 2] = sensores.terreno[i];
        cont++;
      }

      cont = -3; 
      for(int i=9; i<16; ++i){  
        if(mapaResultado[x + cont][y + 3] == '?')
          mapaResultado[x + cont][y + 3] = sensores.terreno[i];
        cont++;
      }

      break;

    case sur:
      cont = -1; 
      for(int i=3; i>0; i--){  
        if(mapaResultado[x + 1][y + cont] == '?')
          mapaResultado[x + 1][y + cont] = sensores.terreno[i];
        cont++;
      }

      cont = -2; 
      for(int i=8; i>3; i--){  
        if(mapaResultado[x + 2][y + cont] == '?')
          mapaResultado[x + 2][y + cont] = sensores.terreno[i];
        cont++;
      }

      cont = -3; 
      for(int i=15; i>8; i--){  
        if(mapaResultado[x + 3][y + cont] == '?')
          mapaResultado[x + 3][y + cont] = sensores.terreno[i];
        cont++;
      }

      break;

    case oeste:
      cont = -1; 
      for(int i=3; i>0; i--){  
        if(mapaResultado[x + cont][y - 1] == '?')
          mapaResultado[x + cont][y - 1] = sensores.terreno[i];
        cont++;
      }

      cont = -2; 
      for(int i=8; i>3; i--){  
        if(mapaResultado[x + cont][y - 2] == '?')
          mapaResultado[x + cont][y - 2] = sensores.terreno[i];
        cont++;
      }

      cont = -3; 
      for(int i=15; i>8; i--){  
        if(mapaResultado[x + cont][y - 3] == '?')
          mapaResultado[x + cont][y - 3] = sensores.terreno[i];
        cont++;
      }

      break;

    case norte:
      cont = -1; 
      for(int i=1; i<4; ++i){  
        if(mapaResultado[x - 1][y + cont] == '?')
          mapaResultado[x - 1][y + cont] = sensores.terreno[i];
        cont++;
      }

      cont = -2; 
      for(int i=4; i<9; ++i){  
        if(mapaResultado[x - 2][y + cont] == '?')
          mapaResultado[x - 2][y + cont] = sensores.terreno[i];
        cont++;
      }

      cont = -3; 
      for(int i=9; i<16; ++i){  
        if(mapaResultado[x - 3][y + cont] == '?')
          mapaResultado[x - 3][y + cont] = sensores.terreno[i];
        cont++;
      }

      break;
  
    default:
      cerr << "\nSe ha producido algún fallo con los sensores" << endl;
      break;
  }
}


int ComportamientoJugador::calcular_costo(estado est, bool zap, bool bik){
	return costo_bateria(mapaResultado[est.fila][est.columna], zap, bik);
}

int ComportamientoJugador::costo_bateria(char tipo, bool zap, bool bik){
	int coste = 1;
	if (tipo == 'A'){
		if (bik)
			coste = 10;
		else
			coste = 100;
	}
	if (tipo == 'B'){
		if (zap)
			coste = 5;
		else
			coste = 50;
	}
	if (tipo == 'T')
		coste = 2;
	if (tipo == 'X')
		coste = 0;

	return coste;
}

void ComportamientoJugador::actualizarBikiniZapatillas(nodo & n){
	int fil = n.st.fila,
	    col = n.st.columna;
	if(mapaResultado[fil][col] == 'K')
		n.bikini = true;
	if(mapaResultado[fil][col] == 'D')
		n.zapatillas = true;
}


// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
