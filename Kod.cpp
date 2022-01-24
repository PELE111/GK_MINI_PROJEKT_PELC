#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

using namespace std;

typedef float point3[3];

static GLfloat viewer[] = { 0.0, 0.0, 10.0 };
// inicjalizacja po³o¿enia obserwatora

static GLfloat thetax = 0.0;   // k¹t obrotu obiektu
static GLfloat thetay = 0.0;   // k¹t obrotu obiektu
static GLfloat thetax1 = 0.0;   // k¹t obrotu obiektu
static GLfloat thetay1 = 0.0;   // k¹t obrotu obiektu

static GLfloat thetax2 = 0.0;   // k¹t obrotu obiektu
static GLfloat thetay2 = 0.0;   // k¹t obrotu obiektu

static GLfloat pix2angle;     // przelicznik pikseli na stopnie
static GLfloat cameraz = 0.0;

static GLint status1 = 0;       // stan klawiszy myszy
static GLint status2 = 0;		// 0 - nie naciœniêto ¿adnego klawisza
								// 1 - naciœniêty zostaæ lewy klawisz

static float x_pos_old = 0;       // poprzednia pozycja kursora myszy
static float y_pos_old = 0;

static float delta_x = 0;        // ró¿nica pomiêdzy pozycj¹ bie¿¹c¹
static float delta_y = 0;			// i poprzedni¹ kursora myszy

static float R1 = 10;

const int N = 51; //rozmiar tablicy wiercho³ków NxN
static GLfloat theta[] = { 0.0, 0.0, 0.0 }; // trzy k¹ty obrotu

//tablice wspó³rzêdnych wierzcho³ków
float x[N][N];
float y[N][N];
float z[N][N];

float Nx[N][N];
float Ny[N][N];
float Nz[N][N];

float t[N][N][2];

float Gspeed=8; //prêdkosæ globalna


//nazwy plików tekstur
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
 // FileName, alokuje pamiêæ i zwraca wskaŸnik (pBits) do bufora w którym
 // umieszczone s¹ dane.
 // Ponadto udostêpnia szerokoœæ (ImWidth), wysokoœæ (ImHeight) obrazu
 // tekstury oraz dane opisuj¹ce format obrazu wed³ug specyfikacji OpenGL
 // (ImComponents) i (ImFormat).
 // Jest to bardzo uproszczona wersja funkcji wczytuj¹cej dane z pliku TGA.
 // Dzia³a tylko dla obrazów wykorzystuj¹cych 8, 24, or 32 bitowy kolor.
 // Nie obs³uguje plików w formacie TGA kodowanych z kompresj¹ RLE.
/*************************************************************************************/

GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat)
{
	/*************************************************************************************/
	// Struktura dla nag³ówka pliku  TGA

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
	// Wartoœci domyœlne zwracane w przypadku b³êdu

	*ImWidth = 0;
	*ImHeight = 0;
	*ImFormat = GL_BGR_EXT;
	*ImComponents = GL_RGB8;


	errno_t err = fopen_s(&pFile, FileName, "rb");
	if (pFile == NULL)
		return NULL;

	/*************************************************************************************/
	// Przeczytanie nag³ówka pliku 

	fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

	/*************************************************************************************/

	// Odczytanie szerokoœci, wysokoœci i g³êbi obrazu

	*ImWidth = tgaHeader.width;
	*ImHeight = tgaHeader.height;
	sDepth = tgaHeader.bitsperpixel / 8;

	/*************************************************************************************/
	// Sprawdzenie, czy g³êbia spe³nia za³o¿one warunki (8, 24, lub 32 bity)

	if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)
		return NULL;
	/*************************************************************************************/

	// Obliczenie rozmiaru bufora w pamiêci
	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	/*************************************************************************************/
	// Alokacja pamiêci dla danych obrazu

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

	float radius; //promieñ
	float distance; //odgleg³óœæ od s³oñca
	float orbit = rand() % 361;  //k¹t orbity
	float speed; //prêdkoœæ oboru wokó³ s³oñca
	float day; //d³uoœæ dnia
	float rotation=0; //k¹t obrotu wokó³ w³asnej osi
	float tilt; //k¹t nachylenia
	bool type; //czy gwiazda
	bool ring; //czy pierœcieñ
	const char* file; //plik tekstury

	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;

	
	void Ring(float R1, float R2) { //funkcja rysuje pierœcieñ wokó³ planety
		float kat, kat1;
		for (int i = 0; i < 359; i++) {
			kat = i * M_PI / 180;
			kat1 = (i + 1) * M_PI / 180;

			glBegin(GL_POLYGON);  //funkcja rysuje wielok¹t
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

		glBegin(GL_POLYGON);  //funkcja rysuje wielok¹t
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

				glBegin(GL_POLYGON);  //funkcja rysuje wielok¹t
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
		float u = 0, v = 0; //zminne u i v wykorzystywane w funkcjach okreœlaj¹cych wierzcho³ki;
		float fN = N; //wartoœæ sta³ej N jako float
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
			u = float(i / (fN -1)); //zmienna zmniejszana proporcjonalnie do sta³ej N, aby znajadowa³a siê w zakresie u¿ytej funkcji [0,1]
			for (int k = 0; k < N; k++) {
				v = float(k / (fN -1)); //zmienna jest zmniejszana proporcjonalnie do sta³ej N, aby znajadowa³a siê w zakresie u¿ytej funkcji [0,1]
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

		if(ring) Ring(1.5 * radius, 2* radius); //rysowanie promieni wokó³ planety

		glPopMatrix();
	}
	void spinPlanet() {  //obrót planety wokó³ s³oñca i w³asnej osi

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
	//odœwie¿enie zawartoœci aktualnego okna
	glutPostRedisplay();

}


GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
// po³o¿enie Ÿród³a œwiat³a
/*************************************************************************************/

/*************************************************************************************/


/*************************************************************************************/
// Funkcja "bada" stan myszy i ustawia wartoœci odpowiednich zmiennych globalnych

void Mouse(int btn, int state, int x, int y) {
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		x_pos_old = x;         // przypisanie aktualnie odczytanej pozycji kursora
		y_pos_old = y;					 // jako pozycji poprzedniej
		status1 = 1;          // wciêniêty zosta³ lewy klawisz myszy
	}

	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {

		x_pos_old = x;         // przypisanie aktualnie odczytanej pozycji kursora
		y_pos_old = y;					 // jako pozycji poprzedniej
		status2 = 1;          // wciêniêty zosta³ lewy klawisz myszy
	}

	else	status1 = status2 = 0;          // nie zosta³ wciêniêty ¿aden klawisz
}

/*************************************************************************************/
// Funkcja "monitoruje" po³o¿enie kursora myszy i ustawia wartoœci odpowiednich
// zmiennych globalnych

void Motion(GLsizei x, GLsizei y) {

	delta_x = x - x_pos_old;     // obliczenie ró¿nicy po³o¿enia kursora myszy

	x_pos_old = x;            // podstawienie bie¿¹cego po³o¿enia jako poprzednie

	delta_y = y - y_pos_old;     // obliczenie ró¿nicy po³o¿enia kursora myszy

	y_pos_old = y;            // podstawienie bie¿¹cego po³o¿enia jako poprzednie

	glutPostRedisplay();     // przerysowanie obrazu sceny
}


// Funkcja okreœlaj¹ca co ma byæ rysowane (zawsze wywo³ywana, gdy trzeba
// przerysowaæ scenê)


void RenderScene(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Czyszczenie okna aktualnym kolorem czyszcz¹cym

	glLoadIdentity();
	// Czyszczenie macierzy bie??cej

	gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// Zdefiniowanie po³o¿enia obserwatora
	// Narysowanie osi przy pomocy funkcji zdefiniowanej powy¿ej

	if (status1 == 1) {						// jeœli lewy klawisz myszy wciêniêty
		thetax += delta_x * pix2angle / 20;	// modyfikacja k¹ta obrotu o kat proporcjonalny do ró¿nicy po³o¿eñ kursora myszy
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
	// Przekazanie poleceñ rysuj¹cych do wykonania
	glutSwapBuffers();
}
/*************************************************************************************/

// Funkcja ustalaj¹ca stan renderowania

void MyInit(void)
{
	/*************************************************************************************/
	// Definicja materia³u z jakiego zrobiony jest czajnik

	GLfloat mat_ambient[] = { 0, 0, 0, 1.0 };
	// wspó³czynniki ka =[kar,kag,kab] dla œwiat³a otoczenia

	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// wspó³czynniki kd =[kdr,kdg,kdb] œwiat³a rozproszonego

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// wspó³czynniki ks =[ksr,ksg,ksb] dla œwiat³a odbitego               

	GLfloat mat_shininess = { 20.0 };
	// wspó³czynnik n opisuj¹cy po³ysk powierzchni


/*************************************************************************************/
// Definicja Ÿród³a œwiat³a


	GLfloat light_position[] = { 0.0, 0.0, 0.0, 0.0 };
	// po³o¿enie Ÿród³a


	GLfloat light_ambient[] = { 0, 0, 0, 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a otoczenia
	// Ia = [Iar,Iag,Iab]

	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a powoduj¹cego
	// odbicie dyfuzyjne Id = [Idr,Idg,Idb]

	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a powoduj¹cego
	// odbicie kierunkowe Is = [Isr,Isg,Isb]

	GLfloat att_constant = { 1.0 };
	// sk³adowa sta³a ds dla modelu zmian oœwietlenia w funkcji
	// odleg³oœci od Ÿród³a

	GLfloat att_linear = { 0.05 };
	// sk³adowa liniowa dl dla modelu zmian oœwietlenia w funkcji
	// odleg³oœci od Ÿród³a

	GLfloat att_quadratic = { 0.001 };
	// sk³adowa kwadratowa dq dla modelu zmian oœwietlenia w funkcji
	// odleg³oœci od Ÿród³a

/*************************************************************************************/
// Ustawienie parametrów materia³u i Ÿród³a œwiat³a

/*************************************************************************************/
// Ustawienie patrametrów materia³u
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

	/*************************************************************************************/
	// Ustawienie parametrów Ÿród³a

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
	

	/*************************************************************************************/
// Definicja Ÿród³a œwiat³a
/*************************************************************************************/
	// Ustawienie opcji systemu oœwietlania sceny

	glShadeModel(GL_SMOOTH); // w³aczenie ³agodnego cieniowania
	glEnable(GL_LIGHTING);   // w³aczenie systemu oœwietlenia sceny
	glEnable(GL_LIGHT0);     // w³¹czenie Ÿród³a o numerze 0

	glEnable(GL_DEPTH_TEST); // w³¹czenie mechanizmu z-bufora

/*************************************************************************************/
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Kolor czyszcz¹cy (wype³nienia okna) ustawiono na czarny

	glEnable(GLUT_FULLY_COVERED);

	//Okreslenie parametrów planet
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
	// W³¹czenie mechanizmu teksturowania
	glEnable(GL_TEXTURE_2D);

	/*************************************************************************************/
	// Ustalenie trybu teksturowania
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/************************************************************************************/
	// Okreœlenie sposobu nak³adania tekstur
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	
}
/*************************************************************************************/
// Funkcja ma za zadanie utrzymanie sta³ych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokoœæ i szerokoœæ okna) s¹ 
// przekazywane do funkcji za ka¿dym razem gdy zmieni siê rozmiar okna.

void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	pix2angle = 360.0 / (float)horizontal;  // przeliczenie pikseli na stopnie

	glMatrixMode(GL_PROJECTION);
	// Prze³¹czenie macierzy bie¿¹cej na macierz projekcji

	glLoadIdentity();
	// Czyszcznie macierzy bie¿¹cej

	gluPerspective(90, 1.0, 1.0, 400.0);
	
	// Ustawienie parametrów dla rzutu perspektywicznego

	
	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
		
	// Ustawienie wielkoœci okna okna widoku (viewport) w zale¿noœci
	// relacji pomiêdzy wysokoœci¹ i szerokoœci¹ okna

	glMatrixMode(GL_MODELVIEW);
	// Prze³¹czenie macierzy bie¿¹cej na macierz widoku modelu  

	glLoadIdentity();
	// Czyszczenie macierzy bie¿¹cej

}

//fynkcja zwortna wyznaczaj¹ prêdkoœæ
void keys(unsigned char key, int x, int y)
{
	if (key == ',' ) Gspeed = Gspeed/2;
	if (Gspeed < 1) Gspeed = 1;
	if (key == '.' ) Gspeed =Gspeed * 2;
	RenderScene(); // przerysowanie obrazu sceny
}

/*************************************************************************************/
// G³ówny punkt wejœcia programu. Program dzia³a w trybie konsoli

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
	// Okreœlenie, ¿e funkcja RenderScene bêdzie funkcj¹ zwrotn¹
	// (callback function).  Bêdzie ona wywo³ywana za ka¿dym razem
	// gdy zajdzie potrzeba przerysowania okna


	glutReshapeFunc(ChangeSize);
	// Dla aktualnego okna ustala funkcjê zwrotn¹ odpowiedzialn¹
	// za zmiany rozmiaru okna                       

	MyInit();
	// Funkcja MyInit() (zdefiniowana powy¿ej) wykonuje wszelkie
	// inicjalizacje konieczne  przed przyst¹pieniem do renderowania
	glEnable(GL_DEPTH_TEST);
	// W³¹czenie mechanizmu usuwania niewidocznych elementów sceny
	glutKeyboardFunc(keys);
	glutMouseFunc(Mouse);
	// Ustala funkcjê zwrotn¹ odpowiedzialn¹ za badanie ruchu myszy
	glutMotionFunc(Motion);

	glutIdleFunc(spin);

	glutMainLoop();
	// Funkcja uruchamia szkielet biblioteki GLUT
}

/*************************************************************************************/
