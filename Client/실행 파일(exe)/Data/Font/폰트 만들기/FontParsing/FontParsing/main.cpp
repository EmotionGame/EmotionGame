////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////


//////////////
// INCLUDES //
//////////////
#include <iostream>
#include <fstream>
using namespace std;


//////////////
// TYPEDEFS //
//////////////

typedef struct
{
	float left, right;
	float top, bottom;
	int sizeX;
	int sizeY;
}FontType;

typedef struct
{
	int x, y;
	int width, height;
}BeforeFontType;

/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
void GetFonttextFilename(char*);
bool CreateAfterFontData(char*);


//////////////////
// MAIN PROGRAM //
//////////////////
int main()
{
	bool result;
	char filename[256];
	char garbage;


	// Read in the name of the model file.
	GetFonttextFilename(filename);

	// Now read the data from the file into the data structures and then output it in our model format.
	result = CreateAfterFontData(filename);
	if (!result)
	{
		return -1;
	}

	// Notify the user the model has been converted.
	cout << "\nFile has been converted." << endl;
	cin >> garbage;

	return 0;
}


void GetFonttextFilename(char* filename)
{
	bool done;
	ifstream fin;


	// Loop until we have a file name.
	done = false;
	while (!done)
	{
		// Ask the user for the filename.
		cout << "Enter model filename: ";

		// Read in the filename.
		cin >> filename;

		// Attempt to open the file.
		fin.open(filename);

		if (fin.good())
		{
			// If the file exists and there are no problems then exit since we have the file name.
			done = true;
		}
		else
		{
			// If the file does not exist or there was an issue opening it then notify the user and repeat the process.
			fin.clear();
			cout << endl;
			cout << "File " << filename << " could not be opened." << endl << endl;
		}
	}

	return;
}



bool CreateAfterFontData(char* filename)
{
	FontType * Font;
	BeforeFontType * BFont;
	ifstream fin;
	char input;
	ofstream fout;
	int XIndex, YIndex, WIndex, HIndex;

	char inputarr[255];
	int I_index = 0;

	// Initialize the Font data structures.
	Font = new FontType[126 - 32 + 1]; // 아스키코드 33~126까지 
	if (!Font)
	{
		return false;
	}

	BFont = new BeforeFontType[126 - 32 + 1];//126 - 33 + 1];
	if (!BFont)
	{
		return false;
	}

	XIndex = 0;
	YIndex = 0;
	WIndex = 0;
	HIndex = 0;

	// Open the file.
	fin.open(filename, ios::binary);

	// Check if it was successful in opening the file.
	if (fin.fail() == true)
	{
		return false;
	}

	fin.get(input);
	while (!fin.eof())
	{
		while (input != '\n' && !fin.eof())
		{
			fin.get(input);
			inputarr[I_index] = input;
			I_index++;
		}


		// 값 넣어주는 함수 실행
		for (int i = 0; i < I_index; i++)
		{

			//x
			if (inputarr[i] == 'x')
			{
				i++;
				// Read in the vertices.
				if (inputarr[i] == '=')
				{
					i++;
					char str[10];
					int strIndex = 0;
					while (inputarr[i] != ' ') {
						str[strIndex] = inputarr[i];
						strIndex++;
						i++;
					}
					str[strIndex] = '\0';
					BFont[XIndex].x = atoi(str);
					XIndex++;
					cout << "x= str : " << str << endl;
				}
			}

			//y
			if (inputarr[i] == 'y')
			{
				i++;
				// Read in the vertices.
				if (inputarr[i] == '=')
				{
					i++;
					char str[10];
					int strIndex = 0;
					while (inputarr[i] != ' ') {
						str[strIndex] = inputarr[i];
						strIndex++;
						i++;
					}
					str[strIndex] = '\0';
					BFont[YIndex].y = atoi(str);
					YIndex++;
					cout << "y= str : " << str << endl;
				}
			}

			//width
			if (inputarr[i] == 't')
			{
				i++;
				// Read in the vertices.
				if (inputarr[i] == 'h')
				{
					i++;
					if (inputarr[i] == '=')
					{
						i++;
						char str[10];
						int strIndex = 0;
						while (inputarr[i] != ' ') {
							str[strIndex] = inputarr[i];
							strIndex++;
							i++;
						}
						str[strIndex] = '\0';
						BFont[WIndex].width = atoi(str);
						WIndex++;
						cout << "th= str : " << str << endl;
					}
				}
			}

			//height
			if (inputarr[i] == 'h')
			{
				i++;
				// Read in the vertices.
				if (inputarr[i] == 't')
				{
					i++;
					if (inputarr[i] == '=')
					{
						i++;
						char str[10];
						int strIndex = 0;
						while (inputarr[i] != ' ') {
							str[strIndex] = inputarr[i];
							strIndex++;
							i++;
						}
						str[strIndex] = '\0';
						BFont[HIndex].height = atoi(str);
						HIndex++;
						cout << "ht= str : " << str << endl;
					}
				}
			}
		}


		if (input == '\n') {
			fin.get(input);
			I_index = 0;
		}
		else
			break;
	}

	// Close the file.
	fin.close();


	///-------정보읽기끝--------////
	cout << "HIndex : " << HIndex << endl;
	int len = strlen(filename);
	filename[len - 4] = '\0';
	strncat_s(filename, 256, ".txt", strlen(".txt"));

	// Open the output file.
	fout.open(filename);

	float textureWidth = 512.0f;
	float textureHeight = 512.0f;
	for (int i = 0; i < 95; i++) //총 95개?
	{
		char iskeycode = i + 32;
		Font[i].sizeX = BFont[i].width;
		Font[i].sizeY = BFont[i].height;
		Font[i].left = (float)(BFont[i].x) / textureWidth;
		Font[i].right = (float)(BFont[i].x + Font[i].sizeX) / textureWidth;
		Font[i].top = (float)(BFont[i].y) / textureHeight;
		Font[i].bottom = (float)(BFont[i].y + Font[i].sizeY) / textureHeight;
		fout << i + 32 << ' ' << iskeycode << ' ' << Font[i].left << ' ' << Font[i].right << ' ' << Font[i].top << ' ' << Font[i].bottom << ' '
			<< Font[i].sizeX << ' ' << Font[i].sizeY;
		fout << endl;
	}

	// Close the output file.
	fout.close();

	// Release the four data structures.
	if (Font)
	{
		delete[] Font;
		Font = 0;
	}

	if (BFont)
	{
		delete[] BFont;
		BFont = 0;
	}
	return true;
}
