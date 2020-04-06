#include "Simulation.h"
#include "Graph.h"

Simulation::Simulation()
{
	maxX = WIDTH ;
	maxY = TAMANIO_PANTALLA_Y;
	foodNum = 0.0;
	alphaSpeed = 0.0;
	smellRadius = 0.0;
	randomJiggleLimit = 0.0;
	babyDeathProb = 0.0;
	grownDeathProb =0.0 ;
	goodDeathProb =0.0 ;
	mode = 0;
	blobNum = 0;
}



Simulation::~Simulation()
{
}

bool Simulation::generateFood(int newFood)
{
	bool res = true;
	for (int i = 0; i < newFood; i++)
	{
		foodPtr[(i)] = new (nothrow) Food(maxX, maxY);
		if (foodPtr[(i)]==nullptr)
		{
			res = false;
		}
	}
	return res;
}

bool Simulation::generateBlobs(int blobNum)
{
	bool result = true;
	double maxSpeed_;
	for (int i = 0; i < blobNum; i++)
	{
		if (mode == 1) {
			maxSpeed_ = maxSpeed;
		}
		else {
			maxSpeed_ = randBetweenReal(0.0, maxSpeed);
		}
		blobPtr[i] = new (nothrow) BabyBlob(maxX, maxY, maxSpeed_, alphaSpeed, smellRadius, babyDeathProb);
		if (blobPtr[i] == nullptr)
		{
			result = false;
		}
	}
	return result;
}




void Simulation::firstData(Graph& myGUI)
{
	maxX = TAMANIO_PANTALLA_X;			//Se complete los valores iniciales en la simulaci�n
	maxY = TAMANIO_PANTALLA_Y-BACKG_Y;
	mode = myGUI.getModo();
	blobNum = myGUI.getBlobNum();
	foodNum = myGUI.getFoodCount();
	maxSpeed = myGUI.getMaxSpeed();
	alphaSpeed = myGUI.getVelp();
	babyDeathProb = myGUI.getDead(0);
	grownDeathProb = myGUI.getDead(1);
	goodDeathProb = myGUI.getDead(2);
	smellRadius = myGUI.getSmellRadius();
	randomJiggleLimit = myGUI.getRJL();
}




void Simulation::getData(Graph& myGUI)	//Recupera los datos que haya modificado el usuario.
{
	int newFood = 0;

	for (int i = 0; i < blobNum; i++)
	{
		if ((blobPtr[i]->getGroup()) == BABYGROUP)		//primero seteo la probabilidad de muerte seg�n grupo etario.
		{
			blobPtr[i]->setDeathStatus(myGUI.getDead(0));
		}

		if ((blobPtr[i]->getGroup()) == GROWNGROUP)
		{
			blobPtr[i]->setDeathStatus(myGUI.getDead(1));
		}

		if ((blobPtr[i]->getGroup()) == GOODOLDGROUP)
		{
			blobPtr[i]->setDeathStatus(myGUI.getDead(2));
		}

		blobPtr[i]->setMaxSpeed(myGUI.getMaxSpeed());		//ajusto la velocidad m�xima, la porcentual, y smellRadius
		blobPtr[i]->setAlphaSpeed(myGUI.getVelp());
		blobPtr[i]->setRadius(myGUI.getSmellRadius());
	}

	newFood = myGUI.getFoodCount();		//agrego o quito comida seg�n corresponda.
	if (newFood >= foodNum)
	{
		generateFood(newFood - foodNum);
	}
	if (newFood < foodNum)
	{
		delFood(foodNum - newFood);
	}
}




void Simulation::blobDeath(void)
{
	for (int i = 0; i < blobNum; i++)

		if (blobPtr[i]->blobFate())	//se decide si un blob debe morir o no.
		{
			clearBlob(i);	//si debe morir, se elimina del arreglo y se ajusta �ste de forma que blobNum sea la primera posici�n libre.
			i--;
		}
}


void Simulation::blobBirth(void)
{
	double maxSpeed_;
	int birthNum = 0;	//cantidad de nacimientos
	for (int i = 0; i < blobNum; i++)
	{
		if (blobPtr[i]->isBlobFull())	//verifica que el blob est� lleno.
		{
			if (mode == 1) {
				maxSpeed_ = maxSpeed; 
			}
			else { 
				maxSpeed_ = randBetweenReal(0.0, maxSpeed); 
			}
			blobPtr[blobNum + birthNum] = new BabyBlob(maxX, maxY, maxSpeed_, alphaSpeed, smellRadius, babyDeathProb);	//si lo est�, se genera un BbyBlob en una posici�n aleatoria.
			birthNum++;
		}
	}
	blobNum += birthNum;	//se ajusta la cantidad de blobs totales en el tablero.
}

void Simulation::clearBlob(int i)
{
	delete blobPtr[i];
	for (int j = i; j<(blobNum - 1); j++)
	{
		blobPtr[j] = blobPtr[j + 1];
	}
	blobNum--;
}

void Simulation::blobMerge(void)
{
	double distance;
	int totalMerges = 0;			//cantidad de merges para luego calcular el promedio de la direcci�n y velocidad.
	for (int i = 0; i < blobNum; i++)
	{
		if (!((blobPtr[i]->getGroup()) == GOODOLDGROUP))		//revisa que el blob no sea uno de tipo GoodOldBlob.
		{
			for (int j = i+1; j < blobNum; j++)				//compara con los otros blobs en el arreglo.
			{
				if ((blobPtr[i]->getGroup()) == (blobPtr[j]->getGroup()))	//revisa que los blobs sean del mismo grupo etario
				{
					distance = getDistance(blobPtr[i]->getX(), blobPtr[j]->getX(), blobPtr[i]->getY(), blobPtr[j]->getY());
					if (distance < ((blobPtr[i]->getcolissionRadius()) / 2.0))
					{					
						blobPtr[i]->setDir((blobPtr[i]->getDir()) + (blobPtr[j]->getDir()));	//la idea es agrupar los datos del nuevo Babyblob en blob[i] para luego generarlo.
						blobPtr[i]->setMaxSpeed((blobPtr[i]->getMaxSpeed()) + (blobPtr[j]->getMaxSpeed()));	//se agrupan direcci�n, velocidad m�xmima, y velocidad relativa.
						blobPtr[i]->setAlphaSpeed((blobPtr[i]->getAlphaSpeed()) + (blobPtr[j]->getAlphaSpeed()));
						totalMerges++;
						clearBlob(j);		//una vez que se recuperaron los datos del blob[j], como en la fusi�n desaparecen los involucrados, podemos eliminar a j ahora
						j--;	//al haberse ajustado el arreglo en clearBlob (disminuyendo en 1 el total), es necesario reducir j en uno para continuar con el for correctamente.
					}
				}
			}
			if (totalMerges > 0) //si hubo alg�n merge se calcula la direcci�n y velocidad del nuevo blob.
			{
				blobPtr[i]->setDir(((blobPtr[i]->getDir()) / totalMerges) + randomJiggleLimit);	//se genera la nueva direcci�n haciendo el promedio y luego sum�ndole randomJiggleLimit;
				blobPtr[i]->setMaxSpeed((blobPtr[i]->getMaxSpeed()) / totalMerges);	//de forma an�loga, se calcula la nueva velocidad m�xima y relativa como el promedio de las anteriores.
				blobPtr[i]->setAlphaSpeed((blobPtr[i]->getAlphaSpeed()) / totalMerges);
				blobPtr[i]->setMergeStatus(true);	//se indica que este blob debe dividirse.
				totalMerges = 0;
			}
		}
	}
	blobDivide();	//genera las divisiones de blobs necesarias.
}

void Simulation::blobDivide(void)
{
	double maxSpeed_;
	double alphaSpeed_;
	for (int i = 0; i < blobNum; i++)
	{
		if (blobPtr[i]->getMergeStatus())	//se fija si es necesario crear un nuevo blob.
		{
			if (mode == 1) {
				maxSpeed_ = blobPtr[i]->getMaxSpeed();
			}
			else {
				maxSpeed_ = randBetweenReal(0.0,blobPtr[i]->getMaxSpeed());
			}
			alphaSpeed_ = blobPtr[i]->getAlphaSpeed();

			switch (blobPtr[i]->getGroup())	//seg�n el grupo de blob, crea un grownblob o un goodoldblob
			{
			case BABYGROUP:
				clearBlob(i);	//el blob que se divide es eliminado, y se agrega otro en la �ltima posici�n del arreglo disponible.
				i--;
				blobPtr[blobNum] = new GrownBlob(maxX, maxY, maxSpeed_, alphaSpeed_, smellRadius, grownDeathProb);
				blobNum++;
				break;
			case GROWNGROUP:
				clearBlob(i);	//De forma similar si se trata de colisiones de GrownBlobs.
				i--;
				blobPtr[blobNum] = new GoodOldBlob(maxX, maxY, maxSpeed_, alphaSpeed_, smellRadius, grownDeathProb);
				blobNum++;
				break;
			default:
				break;
			}
		}
	}
}



void Simulation::delFood(int total)
{	
		int remainder = foodNum - total;
		for (int i = remainder; i < foodNum; i++) 
		{
			delete foodPtr[i];

		}
		foodNum = remainder;
}

void Simulation::gameLoop(void)	//Ciclo de juego
{
	int i = 0;
	//Revisa si alg�n blob debe morir por el fen�meno de blobDeath
	//blobDeath();	

	//Cada blob busca la comida m�s cercana y actualiza su direcci�n, pero No se mueve.
	for (i = 0; i < blobNum; i++)
	{
		blobPtr[i]->check_for_food(foodPtr, foodNum);
	}

	//Los blobs se mueven a su nueva posici�n
	for (i = 0; i < blobNum; i++)
	{
		blobPtr[i]->newPositions(maxX, maxY);
	}

	//Se revisa cu�les blobs est�n en posici�n para comer comida.
	for (i = 0; i < blobNum; i++)
	{
		blobPtr[i]->blobEat(foodPtr, foodNum);
	}

	//Regenera la comida que haya sido consumida por los blobs en una posici�n aleatoria
	for (i = 0; i < foodNum; i++)
	{
		if ((foodPtr[i]->getFoodStatus()) == true)
		{
			foodPtr[i]->newFood(maxX, maxY);
		}
	}

	//Se revisa si hay instancias de blobBirth.
	blobBirth();

	//Revisa si hay colisiones de blobs
	blobMerge();
	
	for (i = 0; i < blobNum; i++)
	{
		blobPtr[i]->moveBlob();
	}

}

