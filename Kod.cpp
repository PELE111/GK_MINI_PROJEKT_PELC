#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

using namespace std;

typedef float point3[3];

static GLfloat viewer[] = { 0.0, 0.0, 10.0 };
// inicjalizacja po�o�enia obserwatora

static GLfloat thetax = 0.0;   // k�t obrotu obiektu
static GLfloat thetay = 0.0;   // k�t obrotu obiektu
static GLfloat thetax1 = 0.0;   // k�t obrotu obiektu
static GLfloat thetay1 = 0.0;   // k�t obrotu obiektu

static GLfloat thetax2 = 0.0;   // k�t obrotu obiektu
static GLfloat thetay2 = 0.0;   // k�t obrotu obiektu

static GLfloat pix2angle;     // przelicznik pikseli na stopnie
static GLfloat cameraz = 0.0;

static GLint status1 = 0;       // stan klawiszy myszy
static GLint status2 = 0;		// 0 - nie naci�ni�to �adnego klawisza
								// 1 - naci�ni�ty zosta� lewy klawisz

static float x_pos_old = 0;       // poprzednia pozycja kursora myszy
static float y_pos_old = 0;

static float delta_x = 0;        // r�nica pomi�dzy pozycj� bie��c�
static float delta_y = 0;			// i poprzedni� kursora myszy

static float R1 = 10;

const int N = 51; //rozmiar tablicy wiercho�k�w NxN
static GLfloat theta[] = { 0.0, 0.0, 0.0 }; // trzy k�ty obrotu

//tablice wsp�rz�dnych wierzcho�k�w
float x[N][N];
float y[N][N];
float z[N][N];

float Nx[N][N];
float Ny[N][N];
float Nz[N][N];

float t[N][N][2];

float Gspeed=8; //pr�dkos� globalna


//nazwy plik�w tekstur
const char tearth[] = "2k_earth_daymap.tga";
const char tmercury[] = "2k_mercury.tga";
const char tvenus[] = "2k_venus_surface.tga";
const char tmars[] = "2k_mars.tga";
const char tjupiter[] = "2k_jupiter.tga";
const char tsaturn[] = "2k_saturn.tga";
const char turanus[] = "2k_uranus.tga";
const char tneptune[] = "2k_neptune.tga";
const char tsun[] = "2k_sun.tga";
const char tstars[] = "2k_stars_milky_way.tga";


/*************************************************************************************/
 // Funkcja wczytuje dane obrazu zapisanego w formacie TGA w pliku o nazwie
 // FileName, alokuje pami�� i zwraca wska�nik (pBits) do bufora w kt�rym
 // umieszczone s� dane.
 // Ponadto udost�pnia szeroko�� (ImWidth), wysoko�� (ImHeight) obrazu
 // tekstury oraz dane opisuj�ce format obrazu wed�ug specyfikacji OpenGL
 // (ImComponents) i (ImFormat).
 // Jest to bardzo uproszczona wersja funkcji wczytuj�cej dane z pliku TGA.
 // Dzia�a tylko dla obraz�w wykorzystuj�cych 8, 24, or 32 bitowy kolor.
 // Nie obs�uguje plik�w w formacie TGA kodowanych z kompresj� RLE.
/*************************************************************************************/

GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat)
{
	/*************************************************************************************/
	// Struktura dla nag��wka pliku  TGA

#pragma pack(1)           
	typedef struct
	{
		GLbyte    idlength;
		GLbyte    colormaptype;
		GLbyte    datatypecode;
		unsigned short    colormapstart;
		unsigned short    colormaplength;
		unsigned char     colormapdepth;
		unsigned short    x_orgin;
		unsigned short    y_orgin;
		unsigned short    width;
		unsigned short    height;
		GLbyte    bitsperpixel;
		GLbyte    descriptor;
	}TGAHEADER;
#pragma pack(8)

	FILE* pFile;
	TGAHEADER tgaHeader;
	unsigned long lImageSize;
	short sDepth;
	GLbyte* pbitsperpixel = NULL;

	/*************************************************************************************/
	// Warto�ci domy�lne zwracane w przypadku b��du

	*ImWidth = 0;
	*ImHeight = 0;
	*ImFormat = GL_BGR_EXT;
	*ImComponents = GL_RGB8;


	errno_t err = fopen_s(&pFile, FileName, "rb");
	if (pFile == NULL)
		return NULL;

	/*************************************************************************************/
	// Przeczytanie nag��wka pliku 

	fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

	/*************************************************************************************/

	// Odczytanie szeroko�ci, wysoko�ci i g��bi obrazu

	*ImWidth = tgaHeader.width;
	*ImHeight = tgaHeader.height;
	sDepth = tgaHeader.bitsperpixel / 8;

	/*************************************************************************************/
	// Sprawdzenie, czy g��bia spe�nia za�o�one warunki (8, 24, lub 32 bity)

	if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)
		return NULL;
	/*************************************************************************************/

	// Obliczenie rozmiaru bufora w pami�ci
	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	/*************************************************************************************/
	// Alokacja pami�ci dla danych obrazu

	pbitsperpixel = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));

	if (pbitsperpixel == NULL)
		return NULL;

	if (fread(pbitsperpixel, lImageSize, 1, pFile) != 1)
	{
		free(pbitsperpixel);
		return NULL;
	}

	/*************************************************************************************/
	// Ustawienie formatu OpenGL
	switch (sDepth)
	{
	case 3:
		*ImFormat = GL_BGR_EXT;
		*ImComponents = GL_RGB8;
		break;
	case 4:
		*ImFormat = GL_BGRA_EXT;
		*ImComponents = GL_RGBA8;
		break;
	case 1:
		*ImFormat = GL_LUMINANCE;
		*ImComponents = GL_LUMINANCE8;
		break;
	};
	fclose(pFile);
	return pbitsperpixel;
}

class Planet {
public:
	float x[N][N];
	float y[N][N];
	float z[N][N];

	float Nx[N][N];
	float Ny[N][N];
	float Nz[N][N];

	float radius; //promie�
	float distance; //odgleg��� od s�o�ca
	float orbit = rand() % 361;  //k�t orbity
	float speed; //pr�dko�� oboru wok� s�o�ca
	float day; //d�uo�� dnia
	float rotation=0; //k�t obrotu wok� w�asnej osi
	float tilt; //k�t nachylenia
	bool type; //czy gwiazda
	bool ring; //czy pier�cie�
	const char* file; //plik tekstury

	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;

	
	void Ring(float R1, float R2) { //funkcja rysuje pier�cie� wok� planety
		float kat, kat1;
		for (int i = 0; i < 359; i++) {
			kat = i * M_PI / 180;
			kat1 = (i + 1) * M_PI / 180;

			glBegin(GL_POLYGON);  //funkcja rysuje wielok�t
			glNormal3f(0, 1, 0);
			glTexCoord2f(i / 360, 0);
			glVertex3f(R1 * cos(kat), 0, R1 * sin(kat));

			glTexCoord2f((i + 1) / 360, 0);
			glVertex3f(R1 * cos(kat1), 0, R1 * sin(kat1));

			glTexCoord2f((i + 1) / 360, 1);
			glVertex3f(R2 * cos(kat1), 0, R2 * sin(kat1));

			glTexCoord2f(i / 360, 1);
			glVertex3f(R2 * cos(kat), 0, R2 * sin(kat));

			glEnd();
		}
		kat = 359 * M_PI / 180;
		kat1 = (0) * M_PI / 180;

		glBegin(GL_POLYGON);  //funkcja rysuje wielok�t
		glTexCoord2f(359 / 360, 0);
		glVertex3f(R1 * cos(kat), 0, R1 * sin(kat));

		glTexCoord2f((0) / 360, 0);
		glVertex3f(R1 * cos(kat1), 0, R1 * sin(kat1));

		glTexCoord2f((0) / 360, 1);
		glVertex3f(R2 * cos(kat1), 0, R2 * sin(kat1));

		glTexCoord2f(359 / 360, 1);
		glVertex3f(R2 * cos(kat), 0, R2 * sin(kat));

		glEnd();
	}


	void PlanetModel() {
		// Zdefiniowanie tekstury 2-D
		glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);

		for (int i = 0; i < N-1 ; i++) {
			for (int k = 0; k < N-1 ; k++) {

				glBegin(GL_POLYGON);  //funkcja rysuje wielok�t
				glNormal3f(Nx[i][k], Ny[i][k], Nz[i][k]);
				glTexCoord2f(t[i][k][0], t[i][k][1]);
				glVertex3f(x[i][k], y[i][k], z[i][k]);

				glNormal3f(Nx[i + 1][k], Ny[i + 1][k], Nz[i + 1][k]);
				glTexCoord2f(t[i + 1][k][0], t[i + 1][k][1]);
				glVertex3f(x[i + 1][k], y[i + 1][k], z[i + 1][k]);

				glNormal3f(Nx[i][k + 1], Ny[i][k + 1], Nz[i][k + 1]);
				glTexCoord2f(t[i][k + 1][0], t[i][k + 1][1]);
				glVertex3f(x[i][k + 1], y[i][k + 1], z[i][k + 1]);
				glEnd();

				glBegin(GL_POLYGON);
				glNormal3f(Nx[i + 1][k + 1], Ny[i + 1][k + 1], Nz[i + 1][k + 1]);
				glTexCoord2f(t[i + 1][k + 1][0], t[i + 1][k + 1][1]);
				glVertex3f(x[i + 1][k + 1], y[i + 1][k + 1], z[i + 1][k + 1]);

				glNormal3f(Nx[i + 1][k], Ny[i + 1][k], Nz[i + 1][k]);
				glTexCoord2f(t[i + 1][k][0], t[i + 1][k][1]);
				glVertex3f(x[i + 1][k], y[i + 1][k], z[i + 1][k]);

				glNormal3f(Nx[i][k + 1], Ny[i][k + 1], Nz[i][k + 1]);
				glTexCoord2f(t[i][k + 1][0], t[i][k + 1][1]);
				glVertex3f(x[i][k + 1], y[i][k + 1], z[i][k + 1]);
				glEnd();

			}
		}
	}

	void initPlanet( ) {
		float u = 0, v = 0; //zminne u i v wykorzystywane w funkcjach okre�laj�cych wierzcho�ki;
		float fN = N; //warto�� sta�ej N jako float
		float l = 0;
		float xu;
		float yu;
		float zu;

		float xv;
		float yv;
		float zv;
	
		//  Przeczytanie obrazu tekstury z pliku

		pBytes = LoadTGAImage(file, &ImWidth, &ImHeight, &ImComponents, &ImFormat);

		for (int i = 0; i < N; i++) {
			u = float(i / (fN -1)); //zmienna zmniejszana proporcjonalnie do sta�ej N, aby znajadowa�a si� w zakresie u�ytej funkcji [0,1]
			for (int k = 0; k < N; k++) {
				v = float(k / (fN -1)); //zmienna jest zmniejszana proporcjonalnie do sta�ej N, aby znajadowa�a si� w zakresie u�ytej funkcji [0,1]
				x[i][k] = radius * sin(2 * M_PI * u) * cos(M_PI * v);
				y[i][k] = -radius * cos(2 * M_PI * u);
				z[i][k] = radius * sin(2 * M_PI * u) * sin(M_PI * v);
				
				Nx[i][k] = x[i][k];
				Ny[i][k] = y[i][k];
				Nz[i][k] = z[i][k];

				l = sqrt(Nx[i][k] * Nx[i][k] + Ny[i][k] * Ny[i][k] + Nz[i][k] * Nz[i][k]);
				Nx[i][k] /= l;
				Ny[i][k] /= l;
				Nz[i][k] /= l;

				if (type) {
					Nx[i][k] *= -1;
					Ny[i][k] *= -1;
					Nz[i][k] *= -1;
				}
				if (x[i][k] == 0 && z[i][k] == 0) {
					Nx[i][k] = 0;
					if (y[i][k] == -5)Ny[i][k] = -1;
					else Ny[i][k] = 1;
					Nz[i][k] = 0;
				}
				
			}
			if(type )Ny[0][i] = 1;
			else Ny[0][i] = -1;
		}
		float fi, fk;
		int i;
		for (int k = 0; k < N; k++) {
			fk = (k);
			fk /= 2;
			for (i = 0; i < N/2+1; i++) {
				fi = (i);
				fi *= 2;

				t[i][k][0] = ((fN-1)/2-fk) / (fN-1);
				t[i][k][1] = (fN-1-fi) / (fN-1);
			};
			
			for (; i < N; i++) {
				fi = (i) -(fN-1)/2;
				fi *= 2;

				t[i][k][0] = (fN-1-fk) / (fN-1);
				t[i][k][1] = (fi) / (fN-1);
			}
		}
	}

	Planet(float _radius,float _distance, float _speed, float _day, bool _type, const char*  _file, float _tilt, bool _ring) {
		
		radius = _radius;
		distance = _distance;
		if (_speed != 0) speed = 1 / _speed;
		else speed = 0;
		day = 1/_day;
		type = _type;
		file = _file;
		tilt = _tilt;
		ring = _ring;
		initPlanet();
	}

	Planet() {}

	void drawPlanet() {
		glPushMatrix();

		glRotatef(orbit, 0, 1, 0);
		glTranslatef(distance, 0.0, 0.0);
		glRotatef(-orbit, 0, 1, 0);
		glRotatef(tilt, 0, 0, 1);
		glRotatef(rotation, 0, 1, 0);

		PlanetModel();

		if(ring) Ring(1.5 * radius, 2* radius); //rysowanie promieni wok� planety

		glPopMatrix();
	}
	void spinPlanet() {  //obr�t planety wok� s�o�ca i w�asnej osi

		orbit -= speed*Gspeed;
		rotation -= day * Gspeed;
		if (orbit <0 ) orbit += 360.0;
		if (rotation <0 ) rotation += 360.0;


	}

};
Planet stars;
Planet sun;
Planet mercury;
Planet venus;
Planet earth;
Planet mars;
Planet jupiter;
Planet saturn;
Planet uranus;
Planet neptune;


void spin() {  //funkcja obraca wszystkie planety
	sun.spinPlanet();
	mercury.spinPlanet();
	venus.spinPlanet();
	earth.spinPlanet();
	mars.spinPlanet();
	jupiter.spinPlanet();
	saturn.spinPlanet();
	uranus.spinPlanet();
	neptune.spinPlanet();
	//od�wie�enie zawarto�ci aktualnego okna
	glutPostRedisplay();

}


GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
// po�o�enie �r�d�a �wiat�a
/*************************************************************************************/

/*************************************************************************************/


/*************************************************************************************/
// Funkcja "bada" stan myszy i ustawia warto�ci odpowiednich zmiennych globalnych

void Mouse(int btn, int state, int x, int y) {
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		x_pos_old = x;         // przypisanie aktualnie odczytanej pozycji kursora
		y_pos_old = y;					 // jako pozycji poprzedniej
		status1 = 1;          // wci�ni�ty zosta� lewy klawisz myszy
	}

	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {

		x_pos_old = x;         // przypisanie aktualnie odczytanej pozycji kursora
		y_pos_old = y;					 // jako pozycji poprzedniej
		status2 = 1;          // wci�ni�ty zosta� lewy klawisz myszy
	}

	else	status1 = status2 = 0;          // nie zosta� wci�ni�ty �aden klawisz
}

/*************************************************************************************/
// Funkcja "monitoruje" po�o�enie kursora myszy i ustawia warto�ci odpowiednich
// zmiennych globalnych

void Motion(GLsizei x, GLsizei y) {

	delta_x = x - x_pos_old;     // obliczenie r�nicy po�o�enia kursora myszy

	x_pos_old = x;            // podstawienie bie��cego po�o�enia jako poprzednie

	delta_y = y - y_pos_old;     // obliczenie r�nicy po�o�enia kursora myszy

	y_pos_old = y;            // podstawienie bie��cego po�o�enia jako poprzednie

	glutPostRedisplay();     // przerysowanie obrazu sceny
}


// Funkcja okre�laj�ca co ma by� rysowane (zawsze wywo�ywana, gdy trzeba
// przerysowa� scen�)


void RenderScene(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Czyszczenie okna aktualnym kolorem czyszcz�cym

	glLoadIdentity();
	// Czyszczenie macierzy bie??cej

	gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// Zdefiniowanie po�o�enia obserwatora
	// Narysowanie osi przy pomocy funkcji zdefiniowanej powy�ej

	if (status1 == 1) {						// je�li lewy klawisz myszy wci�ni�ty
		thetax += delta_x * pix2angle / 20;	// modyfikacja k�ta obrotu o kat proporcjonalny do r�nicy po�o�e� kursora myszy
		thetay += delta_y * pix2angle / 20;
		if (thetay > M_PI / 2 - 0.000001) {
			thetay = M_PI / 2 - 0.000001;
		}
		else if (thetay < -M_PI / 2 + 0.000001) {
			thetay = -M_PI / 2 + 0.000001;
		}
	}
	else if (status2 == 1) {
		R1 += delta_y / 10;
		if (R1 < sun.radius+1) R1 = sun.radius+1;
		else if (R1 > 100) R1 = 100;
	}

	viewer[0] = R1 * cos(thetax) * cos(thetay);
	viewer[1] = R1 * sin(thetay);
	viewer[2] = R1 * sin(thetax) * cos(thetay);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glColor3f(1.0f, 1.0f, 1.0f);

	stars.drawPlanet();
	sun.drawPlanet();
	mercury.drawPlanet();
	venus.drawPlanet();
	earth.drawPlanet();
	mars.drawPlanet();
	jupiter.drawPlanet();
	saturn.drawPlanet();
	uranus.drawPlanet();
	neptune.drawPlanet();


	glFlush();
	// Przekazanie polece� rysuj�cych do wykonania
	glutSwapBuffers();
}
/*************************************************************************************/

// Funkcja ustalaj�ca stan renderowania

void MyInit(void)
{
	/*************************************************************************************/
	// Definicja materia�u z jakiego zrobiony jest czajnik

	GLfloat mat_ambient[] = { 0, 0, 0, 1.0 };
	// wsp�czynniki ka =[kar,kag,kab] dla �wiat�a otoczenia

	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki kd =[kdr,kdg,kdb] �wiat�a rozproszonego

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki ks =[ksr,ksg,ksb] dla �wiat�a odbitego               

	GLfloat mat_shininess = { 20.0 };
	// wsp�czynnik n opisuj�cy po�ysk powierzchni


/*************************************************************************************/
// Definicja �r�d�a �wiat�a


	GLfloat light_position[] = { 0.0, 0.0, 0.0, 0.0 };
	// po�o�enie �r�d�a


	GLfloat light_ambient[] = { 0, 0, 0, 1.0 };
	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a otoczenia
	// Ia = [Iar,Iag,Iab]

	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a powoduj�cego
	// odbicie dyfuzyjne Id = [Idr,Idg,Idb]

	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a powoduj�cego
	// odbicie kierunkowe Is = [Isr,Isg,Isb]

	GLfloat att_constant = { 1.0 };
	// sk�adowa sta�a ds dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

	GLfloat att_linear = { 0.05 };
	// sk�adowa liniowa dl dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

	GLfloat att_quadratic = { 0.001 };
	// sk�adowa kwadratowa dq dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

/*************************************************************************************/
// Ustawienie parametr�w materia�u i �r�d�a �wiat�a

/*************************************************************************************/
// Ustawienie patrametr�w materia�u
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

	/*************************************************************************************/
	// Ustawienie parametr�w �r�d�a

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
	

	/*************************************************************************************/
// Definicja �r�d�a �wiat�a
/*************************************************************************************/
	// Ustawienie opcji systemu o�wietlania sceny

	glShadeModel(GL_SMOOTH); // w�aczenie �agodnego cieniowania
	glEnable(GL_LIGHTING);   // w�aczenie systemu o�wietlenia sceny
	glEnable(GL_LIGHT0);     // w��czenie �r�d�a o numerze 0

	glEnable(GL_DEPTH_TEST); // w��czenie mechanizmu z-bufora

/*************************************************************************************/
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Kolor czyszcz�cy (wype�nienia okna) ustawiono na czarny

	glEnable(GLUT_FULLY_COVERED);

	//Okreslenie parametr�w planet
	stars = Planet(200, 0, 0, 0, true, tstars, 0, false);
	sun = Planet(5, 0, 0, 25, true, tsun, 7.25, false);
	mercury = Planet(1, 10,58.5, 88, false, tmercury, 0.03, false); 
	venus = Planet(1, 18, 224,243, false, tvenus, 2.64, false);
	earth = Planet(1, 25, 365,1, false, tearth, 23.44, false);
	mars = Planet(1, 30, 686,1,false, tmars, 25.19, false);
	jupiter = Planet(3, 40, 4333,0.41, false, tjupiter, 3.13, false);
	saturn = Planet(2, 50, 10765,0.45, false, tsaturn, 26.73, true);
	uranus = Planet(2, 60, 30707,0.7, false, turanus, 82.23, false);
	neptune = Planet(2, 70, 60223,0.66, false, tneptune, 28.32, false);
/*************************************************************************************/
	// W��czenie mechanizmu teksturowania
	glEnable(GL_TEXTURE_2D);

	/*************************************************************************************/
	// Ustalenie trybu teksturowania
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/************************************************************************************/
	// Okre�lenie sposobu nak�adania tekstur
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	
}
/*************************************************************************************/
// Funkcja ma za zadanie utrzymanie sta�ych proporcji rysowanych
// w przypadku zmiany rozmiar�w okna.
// Parametry vertical i horizontal (wysoko�� i szeroko�� okna) s� 
// przekazywane do funkcji za ka�dym razem gdy zmieni si� rozmiar okna.

void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	pix2angle = 360.0 / (float)horizontal;  // przeliczenie pikseli na stopnie

	glMatrixMode(GL_PROJECTION);
	// Prze��czenie macierzy bie��cej na macierz projekcji

	glLoadIdentity();
	// Czyszcznie macierzy bie��cej

	gluPerspective(90, 1.0, 1.0, 400.0);
	
	// Ustawienie parametr�w dla rzutu perspektywicznego

	
	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
		
	// Ustawienie wielko�ci okna okna widoku (viewport) w zale�no�ci
	// relacji pomi�dzy wysoko�ci� i szeroko�ci� okna

	glMatrixMode(GL_MODELVIEW);
	// Prze��czenie macierzy bie��cej na macierz widoku modelu  

	glLoadIdentity();
	// Czyszczenie macierzy bie��cej

}

//fynkcja zwortna wyznaczaj� pr�dko��
void keys(unsigned char key, int x, int y)
{
	if (key == ',' ) Gspeed = Gspeed/2;
	if (Gspeed < 1) Gspeed = 1;
	if (key == '.' ) Gspeed =Gspeed * 2;
	RenderScene(); // przerysowanie obrazu sceny
}

/*************************************************************************************/
// G��wny punkt wej�cia programu. Program dzia�a w trybie konsoli

void main(void)
{
	cout << "Lewy przycisk myszy - obrot kamery" << endl
		<< "Prawy przycisk myszy - przyblizenie kamery" << endl;
	cout << "Przyciski: " << endl
		<< ", - zwolnij" << endl
		<< ". - przyspiesz" << endl;

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(1000, 1000);

	glutCreateWindow("Rzutowanie perspektywiczne");

	glutDisplayFunc(RenderScene);
	// Okre�lenie, �e funkcja RenderScene b�dzie funkcj� zwrotn�
	// (callback function).  B�dzie ona wywo�ywana za ka�dym razem
	// gdy zajdzie potrzeba przerysowania okna


	glutReshapeFunc(ChangeSize);
	// Dla aktualnego okna ustala funkcj� zwrotn� odpowiedzialn�
	// za zmiany rozmiaru okna                       

	MyInit();
	// Funkcja MyInit() (zdefiniowana powy�ej) wykonuje wszelkie
	// inicjalizacje konieczne  przed przyst�pieniem do renderowania
	glEnable(GL_DEPTH_TEST);
	// W��czenie mechanizmu usuwania niewidocznych element�w sceny
	glutKeyboardFunc(keys);
	glutMouseFunc(Mouse);
	// Ustala funkcj� zwrotn� odpowiedzialn� za badanie ruchu myszy
	glutMotionFunc(Motion);

	glutIdleFunc(spin);

	glutMainLoop();
	// Funkcja uruchamia szkielet biblioteki GLUT
}

/*************************************************************************************/
