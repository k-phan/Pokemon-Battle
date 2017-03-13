#include <grlib.h>

const double type_advantage[17][17] =
{
	{1,1,1,1,1,0.5,1,0,0.5,1,1,1,1,1,1,1,1},
	{2,1,0.5,0.5,1,2,0.5,0,2,1,1,1,1,0.5,2,1,2},
	{1,2,1,1,1,0.5,2,1,0.5,1,1,2,0.5,1,1,1,1},
	{1,1,1,0.5,0.5,0.5,1,0.5,0,1,1,2,1,1,1,1,1},
	{1,1,0,2,1,2,0.5,1,2,2,1,0.5,2,1,1,1,1},
	{1,0.5,2,1,0.5,1,2,1,0.5,2,1,1,1,1,2,1,1},
	{1,0.5,0.5,0.5,1,1,1,0.5,0.5,0.5,1,2,1,2,1,1,2},
	{0,1,1,1,1,1,1,2,0.5,1,1,1,1,2,1,1,0.5},
	{1,1,1,1,1,2,1,1,0.5,0.5,0.5,1,0.5,1,2,1,1},
	{1,1,1,1,1,0.5,2,1,2,0.5,0.5,2,1,1,2,0.5,1},
	{1,1,1,1,2,2,1,1,1,2,0.5,0.5,1,1,1,0.5,1},
	{1,1,0.5,0.5,2,2,0.5,1,0.5,0.5,2,0.5,1,1,1,0.5,1},
	{1,1,2,1,0,1,1,1,1,1,2,0.5,0.5,1,1,0.5,1},
	{1,2,1,2,1,1,1,1,0.5,1,1,1,1,0.5,1,1,0},
	{1,1,2,1,2,1,1,1,0.5,0.5,0.5,2,1,1,0.5,2,1},
	{1,1,1,1,1,1,1,1,0.5,1,1,1,1,1,1,2,1},
	{1,0.5,1,1,1,1,1,2,0.5,1,1,1,1,2,1,1,0.5},
};

double calcDamage(double atkPwr, double atk, double def, int atkType, int defType1, int defType2)
{
	double damage = atkPwr * atk/100 * def/100 * type_advantage[atkType][defType1];
	if(defType2 != 20)
		return damage * type_advantage[atkType][defType2];
	else
		return damage;
}

double calcEff(int atkType, int defType1, int defType2)
{
	if(defType2 != 20)
		return type_advantage[atkType][defType1] * type_advantage[atkType][defType2];
	else
		return type_advantage[atkType][defType1];
}
