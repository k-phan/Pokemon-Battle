#define NRM 0
#define FGT 1
#define FLY 2
#define PSN 3
#define GND 4
#define RCK 5
#define BUG 6
#define GHS 7
#define STL 8
#define FIR 9
#define WTR 10
#define GRS 11
#define ELC 12
#define PSY 13
#define ICE 14
#define DRG 15
#define DRK 16

#include <grlib.h>

double calcDamage(double atkPwr, double atk, double def, int atkType, int defType1, int defType2);
double calcEff(int atkType, int defType1, int defType2);
void draw_aggron(Graphics_Context * gs, int x, int y);
void draw_gengar(Graphics_Context * gs, int x, int y);
void draw_gyrados(Graphics_Context * gs, int x, int y);
void draw_shiftry(Graphics_Context * gs, int x, int y);
void draw_swellow(Graphics_Context * gs, int x, int y);
void draw_machoke(Graphics_Context * gs, int x, int y);
void draw_metagross(Graphics_Context * gs, int x, int y);
void draw_jolteon(Graphics_Context * gs, int x, int y);
void draw_typhlosion(Graphics_Context * gs, int x, int y);
void draw_swampert(Graphics_Context * gs, int x, int y);
void draw_title(Graphics_Context * gs, int x, int y);

void selectScreenDraw(int pokemon, Graphics_Context * gs, int x, int y)
{
	switch(pokemon)
	{
	case 0:
		draw_aggron(gs, x, y);
		break;
	case 1:
		draw_gengar(gs, x, y);
		break;
	case 2:
		draw_gyrados(gs, x, y);
		break;
	case 3:
		draw_shiftry(gs, x, y);
		break;
	case 4:
		draw_swellow(gs, x, y);
		break;
	case 5:
		draw_machoke(gs, x, y);
		break;
	case 6:
		draw_metagross(gs, x, y);
		break;
	case 7:
		draw_jolteon(gs, x, y);
		break;
	case 8:
		draw_typhlosion(gs, x, y);
		break;
	case 9:
		draw_swampert(gs, x, y);
		break;
	default: break;
	}
}

const char *pokemon_list[][8] =
{
	/* POKEMON / TYPES / HP (100-200) / ATK (0-100) / DEF (0-100) */
	{"Aggron", "STL", "GND", "185", "50", "60", "8", "4"},
	{"Gengar", "GST", "PSN", "120", "70", "30", "7", "3"},
	{"Gyrados", "WTR", "DRK", "190", "60", "45", "10", "16"},
	{"Shiftry", "GRS", "DRK", "145", "55", "30", "11", "16"},
	{"Swellow", "FLY", NULL, "145", "50", "40", "2", "20"},
	{"Machoke", "FGT", NULL, "165", "75", "45", "1", "20"},
	{"Metagross", "STL", "PSY", "175", "65", "45", "8", "13"},
	{"Jolteon", "ELC", NULL, "130", "70", "40", "12", "20"},
	{"Typhlosion", "FIR", NULL, "140", "60", "35", "9", "20"},
	{"Swampert", "WTR", "GND", "160", "50", "55", "10", "4"},
	{"Magikarp", "DRG", "STL", "100", "10", "10", "15", "8"}
};

const char *move_list[][4] =
{
	// Aggron
	{"Earthq.", "GND", "90", "4"},
	{"Iron Tail", "STL", "70", "8"},
	{"Bite", "DRK", "60", "16"},
	{"Hyper B.", "NRM", "120", "0"},

	// Gengar
	{"Shadow B.", "DRK", "70", "16"},
	{"Sludge B.", "PSN", "70", "3"},
	{"Psychic", "PSY", "90", "13"},
	{"Thunder", "ELC", "100", "12"},

	// Gyrados
	{"Crunch", "DRK", "80", "16"},
	{"Hydro P.", "WTR", "120", "10"},
	{"Thrash", "NRM", "90", "0"},
	{"Ice Fang", "ICE", "65", "14"},

	// Shiftry
	{"Hurricane", "FLY", "90", "2"},
	{"Razor L.", "GRS", "60", "11"},
	{"Icy Wind", "ICE", "45", "14"},
	{"Feint A.", "DRK", "65", "16"},

	// Swellow
	{"Brave B.", "FLY", "80", "2"},
	{"Steel W.", "STL", "65", "8"},
	{"Heat Wave", "FIR", "120", "9"},
	{"Boomburst", "NRM", "55", "0"},

	// Machoke
	{"Seis. T.", "FGT", "50", "1"},
	{"Knock Off", "DRK", "60", "16"},
	{"Dual Chop", "FGT", "80", "1"},
	{"Fire Pun.", "FIR", "75", "9"},

	// Metagross
	{"Confusion", "PSY", "60", "13"},
	{"Metal C.", "STL", "65", "8"},
	{"Thun. P.", "ELC", "75", "12"},
	{"Dyn. Pun.", "FGT", "100", "1"},

	// Jolteon
	{"Sand Att.", "GND", "50", "4"},
	{"Thun. F.", "ELC", "80", "12"},
	{"Double K.", "FGT", "65", "1"},
	{"Tackle", "NRM", "35", "0"},

	// Typhlosion
	{"Double E.", "NRM", "100", "0"},
	{"Fire B." , "FIR", "120", "9"},
	{"Focus P.", "FGT", "100", "1"},
	{"Shadow C.", "GST", "70", "7"},

	// Swampert
	{"Mudshot", "GND", "80", "4"},
	{"Surf", "WTR", "90", "10"},
	{"Avalanche", "ICE", "100", "14"},
	{"Anc. Pwr", "RCK", "65", "5"},

	// Magikarp
	{"Flail", "NRM", "0", "0"},
	{"Tackle", "NRM", "35", "0"},
	{"Splash", "NRM", "0", "0"},
	{"Bounce", "FLY", "0", "2"}
};
