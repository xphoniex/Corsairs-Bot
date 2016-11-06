// bot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <WinBase.h>
#include "EasyBMP.h"
#include <cstdlib>
#include <vector>
#include <queue>
#include <string>
#include <cmath>
#include <string>

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define HRCHECK(__expr) {hr=(__expr);if(FAILED(hr)){wprintf(L"FAILURE 0x%08X (%i)\n\tline: %u file: '%s'\n\texpr: '" WIDEN(#__expr) L"'\n",hr, hr, __LINE__,__WFILE__);goto cleanup;}}
#define RELEASE(__p) {if(__p!=nullptr){__p->Release();__p=nullptr;}}

#define PI 3.14159265
#define BALLPIXELS 360
#define errorMargin 25
#define DEVELOPMENT 0

double deg = 0;
int time = 0;
bool CW = true; //clock-wise
bool playing = false;
int LEVEL = 1;
double BALL_V = 395; // pixel/sec
double SHIP_V = 99;  //     °/sec


int R = 342;
int D = 290;
int OX = 534,
	OY = 483;
int SHIP_OX = 530,
	SHIP_OY = 140;

int TAP_SLEEP = 135;
int LAST_TAP = 0;

int LASTNUMLOCK = 0,
	LASTR = 0;

short map[660][660] = { {} };

D3DLOCKED_RECT rc;

using namespace std;

struct ball
{
	int x,y;
};

bool SCREEN[1600][900] = { {} };

inline bool hit(int ballX, int ballY, int shipX, int shipY)
{
	//ball | x -> x+35 
	//	   | y -> y+35

	//ship | x-32 -> x+32 
	//	   | y-35 -> y+35
	bool X = false;
	bool Y = false;

	if (ballX >= shipX-32-errorMargin && ballX <= shipX+32+errorMargin)
		X = true;
	
	if (ballX+35 >= shipX-32-errorMargin && ballX+35 <= shipX+32+errorMargin)
		X = true;

	if (ballY >= shipY-32-errorMargin && ballY <= shipY+32+errorMargin)
		Y = true;

	if (ballY+35 >= shipY-32-errorMargin && ballY+35 <= shipY+32+errorMargin)
		Y = true;

	if (X && Y)
		return true;
	else
		return false;
}

DWORD WINAPI Screenshot(LPVOID lpParam)
{
	int now,
		tap_count = 0;
	UINT adapter = 0;
	UINT count = 1;
	UINT i = 0;

	int sumFirstColumn,
		sumFirstRow,
		sumLastPlusOneColumn,
		sumLastPlusOneRow;

	ball tmp;

	BMP test;
	test.SetSize(1600,900);
	test.SetBitDepth(32);

    HRESULT hr = S_OK;
    IDirect3D9 *d3d = nullptr;
    IDirect3DDevice9 *device = nullptr;
    IDirect3DSurface9 *surface = nullptr;
    D3DPRESENT_PARAMETERS parameters = { 0 };
    D3DDISPLAYMODE mode;
    
    UINT pitch;
    SYSTEMTIME st;
    //LPBYTE *shots = nullptr;

    // init D3D and get screen size
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    HRCHECK(d3d->GetAdapterDisplayMode(adapter, &mode));

    parameters.Windowed = TRUE;
    parameters.BackBufferCount = 1;
    parameters.BackBufferHeight = mode.Height;
    parameters.BackBufferWidth = mode.Width;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    parameters.hDeviceWindow = NULL;

    // create device & capture surface
    HRCHECK(d3d->CreateDevice(adapter, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &device));
    HRCHECK(device->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, nullptr));

    // compute the required buffer size
    HRCHECK(surface->LockRect(&rc, NULL, 0));
    pitch = rc.Pitch;
    HRCHECK(surface->UnlockRect());

    // allocate screenshots buffers
//    shots = new LPBYTE[count];
//    for (UINT i = 0; i < count; i++)
//    {
//      shots[i] = new BYTE[pitch * mode.Height];
//    }

    //GetSystemTime(&st); // measure the time we spend doing <count> captures
    //wprintf(L"%i:%i:%i.%i\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);	

	while (true)
	{
	if (playing)
	{
		++i;
		GetSystemTime(&st);
		//if (i<55)
		wprintf(L"Shotting #%i    %i:%i:%i.%i", i, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		double current_deg = deg;
		int current_time = time;

		queue <ball> prospects;
		vector<ball> balls;
		
		// get the data
		HRCHECK(device->GetFrontBufferData(0, surface));

		// copy it into our buffers
		HRCHECK(surface->LockRect(&rc, NULL, 0));
		
		BYTE *bytePointer=(BYTE*)rc.pBits;

		for (DWORD y=0;y<mode.Height;y++)
		{
			for (DWORD x=0;x<mode.Width;x++)
			{
				DWORD index=(x*4+(y*(rc.Pitch)));
			
				/*
				BYTE b=bytePointer[index]; // Blue
				BYTE g=bytePointer[index+1]; // Green
				BYTE r=bytePointer[index+2]; // Red
				BYTE a=bytePointer[index+3]; // Alpha

				test(x,y)->Red = (int)r;
				test(x,y)->Green = (int)g;
				test(x,y)->Blue = (int)b;
				test(x,y)->Alpha = (int)a;
				*/
				
				if (DEVELOPMENT)
				{
				test(x,y)->Red = (int)bytePointer[index+2];
				test(x,y)->Green = (int)bytePointer[index+1];
				test(x,y)->Blue = (int)bytePointer[index];
				}

				if ((int)bytePointer[index+2]==77 && (int)bytePointer[index+1]==77 && (int)bytePointer[index]==77)
					SCREEN[x][y] = true;
				else
					SCREEN[x][y] = false;
			}
		}
		
		if (DEVELOPMENT)
		{
		char name0[20] = {};
		char ii0[5];
		strcat(name0,"o-live");
		itoa(i,ii0,10);
		strcat(name0,ii0);
		strcat(name0,".bmp");
		test.WriteToFile(name0);
		}

		int y_at = 0;
	
		GetSystemTime(&st);
		//if (i<55)
		wprintf(L"\nIndexing...      %i:%i:%i.%i\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		//first 35x35
		for (int x=186;x<221;++x)
			for (int y=135;y<170;++y)
				if (SCREEN[x][y])
				//if ((int)test(x,y)->Red==77 && (int)test(x,y)->Green==77 && (int)test(x,y)->Blue==77)
					++map[0][0];

		if (map[0][0]>BALLPIXELS)
		{
			tmp.x = 0;
			tmp.y = 0;
			prospects.push(tmp);
		}

completeTheRow:
		//complete row
		for (int x=1;x<660;++x)
		{
			if (x>255 && x<406)
				if (y_at>255 && y_at<406) 
					continue;
		
			sumFirstColumn = 0;
			sumLastPlusOneColumn = 0;
			for (int y=135+y_at;y<170+y_at;++y)
			{
				//if ((int)test(x+185,y)->Red==77 && (int)test(x+185,y)->Green==77 && (int)test(x+185,y)->Blue==77)
				if (SCREEN[x+185][y])
					++sumFirstColumn;
				//if ((int)test(x+220,y)->Red==77 && (int)test(x+220,y)->Green==77 && (int)test(x+220,y)->Blue==77)
				if (SCREEN[x+220][y])	
					++sumLastPlusOneColumn;			
			}
			map[x][y_at] = map[x-1][y_at] - sumFirstColumn + sumLastPlusOneColumn;
		
			if (map[x][y_at]>BALLPIXELS)
			{
				tmp.x = x;
				tmp.y = y_at;
				prospects.push(tmp);
			}
		}

		++y_at;

		if (y_at==659) goto detect_balls;

		for (int x=186;x<221;++x)
		{
			sumFirstRow = 0;
			sumLastPlusOneRow = 0;
			//if ((int)test(x,y_at+134)->Red==77 && (int)test(x,y_at+134)->Green==77 && (int)test(x,y_at+134)->Blue==77)
			if (SCREEN[x][y_at+134])	
				++sumFirstRow;
			//if ((int)test(x,y_at+169)->Red==77 && (int)test(x,y_at+169)->Green==77 && (int)test(x,y_at+169)->Blue==77)
			if (SCREEN[x][y_at+169])	
				++sumLastPlusOneRow;			
		}

		map[0][y_at] = map[0][y_at-1] - sumFirstRow + sumLastPlusOneRow;
	
		if (map[0][y_at]>BALLPIXELS)
		{
			tmp.x = 0;
			tmp.y = y_at;
			prospects.push(tmp);
		}

		if (y_at<660) goto completeTheRow;

detect_balls:

		while (!prospects.empty())	
		{
			int x = prospects.front().x;
			int y = prospects.front().y;

calibration:

			if (x!=659)
			if (map[x+1][y]>map[x][y]) { ++x; goto calibration; }
			if (x!=0)
			if (map[x-1][y]>map[x][y]) { --x; goto calibration; }
			if (y!=659)
			if (map[x][y+1]>map[x][y]) { ++y; goto calibration; }
			if (y!=0)
			if (map[x][y-1]>map[x][y]) { --y; goto calibration; }

			for (size_t j=0;j<balls.size();++j)
			{
				if (abs(x+186-balls[j].x)<=35 && abs(y+135-balls[j].y)<=35)
				{
						prospects.pop(); 
						goto detect_balls; 
				}
			}

			tmp.x = x+186;
			tmp.y = y+135;
//			tmp.v = 0;
//			tmp.xv = 0;
//			tmp.yv = 0;
				
			double distanceFromO = sqrt(pow(double(tmp.x+17.5-OX),2)+pow(double(tmp.y+17.5-OY),2))*D/R;
			double timeLeft = (D-distanceFromO)/BALL_V;
			double fullTime = D/BALL_V;
			double spentTime = fullTime - timeLeft;
			double xVelocity = (tmp.x+17.5-OX)/spentTime;
			double yVelocity = (tmp.y+17.5-OY)/spentTime;
			int finalX = xVelocity*timeLeft + tmp.x;
			int finalY = yVelocity*timeLeft + tmp.y;
		
			// SPACESHIP will be @
			// if going CCW
			
			double degreeCCW = SHIP_V * timeLeft;
			if (CW) degreeCCW *= -1;
			
			double deltaX = R * sin ((degreeCCW+current_deg)*PI/180);
			double deltaY = R - (R * cos ((degreeCCW+current_deg)*PI/180));
			
			//double deltaX = R * sin ((degreeCCW+deg)*PI/180);
			//double deltaY = R - (R * cos ((degreeCCW+deg)*PI/180));
		
			int nextX = SHIP_OX-deltaX; int nextY = SHIP_OY + deltaY;

			
			//if (i<55){
			cout << "+new ball @ (" << tmp.x << "," << tmp.y << ") \n";
			cout << "|__ it'll hit @ (" << finalX << "," << finalY << ") within " << timeLeft << "seconds\n";
			cout << "|__ we'll be  @ (" << nextX << "," << nextY << ") (" << current_deg << "° -> " << current_deg+degreeCCW << "°) t=" << current_time <<"\n";
			cout << "hit?=" << hit(finalX,finalY,nextX,nextY) << "\n";
			//}

			if (hit(finalX,finalY,nextX,nextY))
			{
				if (playing)
				{
				SetCursorPos(1300,489);
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				//SetCursorPos(1300,700);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				CW = !CW;

				/*if (CW)
					deg -= 3;
				else
					deg += 3;*/
					
				GetSystemTime(&st);

				now = st.wMilliseconds + (1000 * st.wSecond) + (1000 * 60 * st.wMinute) + (1000 * 60 * 60 * st.wHour);
				if (now-LAST_TAP <= 200)
					++tap_count;
				else
					tap_count = 0;

				LAST_TAP = now;

				if (tap_count == 3)
				{
					cout << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n ";
					tap_count = 0;
					Sleep(400);
					goto changedDirection;
					
				//	if (LEVEL<6)
				//		Sleep(50);
				}
				

				Sleep(TAP_SLEEP);
				
				SetCursorPos(1300,489);
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				//SetCursorPos(1300,700);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				CW = !CW;

				/*if (CW)
					deg -= 3;
				else
					deg += 3;*/

				

				}
				//SetCursorPos(1310,200);
				
				//goto changedDirection;
			}

			balls.push_back(tmp);

			if (DEVELOPMENT)
			{
			for (int j=0;j<35;++j)
			{
				test(tmp.x,tmp.y+j)->Red = 255;
				test(tmp.x,tmp.y+j)->Green = 0;
				test(tmp.x,tmp.y+j)->Blue = 0;
				//test(tmp.x,tmp.y+j)->Alpha = 255;
									   
				test(tmp.x+35,tmp.y+j)->Red = 255;
				test(tmp.x+35,tmp.y+j)->Green = 0;
				test(tmp.x+35,tmp.y+j)->Blue = 0;
				//test(tmp.x+35,tmp.y+j)->Alpha = 255;
			}

			for (int j=0;j<35;++j)
			{
				test(tmp.x+j,tmp.y)->Red = 255;
				test(tmp.x+j,tmp.y)->Green = 0;
				test(tmp.x+j,tmp.y)->Blue = 0;
				//test(tmp.x+j,tmp.y)->Alpha = 255;

				test(tmp.x+j,tmp.y+35)->Red = 255;
				test(tmp.x+j,tmp.y+35)->Green = 0;
				test(tmp.x+j,tmp.y+35)->Blue = 0;
				//test(tmp.x+j,tmp.y+35)->Alpha = 255;
			}
			}
				
		}
changedDirection:

		GetSystemTime(&st);
		//if (i<55)
		wprintf(L"Found all.     %i:%i:%i.%i\n===========================\n\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);	
//		
		if (DEVELOPMENT)
		{
		char name[20] = {};
		char ii[5];
		strcat(name,"live");
		itoa(i,ii,10);
		strcat(name,ii);
		strcat(name,".bmp");

		double curdeltaX = R * sin ((current_deg)*PI/180);
		double curdeltaY = R - (R * cos ((current_deg)*PI/180));
			
		double curX = SHIP_OX-curdeltaX; double curY = SHIP_OY + curdeltaY;

		double shipxv = (curX - OX)/343;
		double shipyv = (curY - OY)/343;
		int Ishipxv, Ishipyv;
		for (int j=1;j<=343;++j)
		{
			Ishipxv = shipxv * j + OX;
			Ishipyv = shipyv * j + OY;
			//cout << "(" << Ishipxv << "," << Ishipyv << ") <- line\n";
			test(Ishipxv,Ishipyv)->Red = 0;
			test(Ishipxv,Ishipyv)->Green = 100;
			test(Ishipxv,Ishipyv)->Blue = 100;
			//test(Ishipxv,Ishipyv)->Alpha = 255;
		}	

		test.WriteToFile(name);
		}
		HRCHECK(surface->UnlockRect());
	}
	}

    // save all screenshots
    //for (UINT i = 0; i < count; i++)
    //{
    //  WCHAR file[100];
    //  wsprintf(file, L"cap%i.png", i);
    //  HRCHECK(SavePixelsToFile32bppPBGRA(mode.Width, mode.Height, pitch, shots[i], file, GUID_ContainerFormatPng));
    //}

  cleanup:
  //  if (shots != nullptr)
  //  {
  //    for (UINT i = 0; i < count; i++)
  //    {
  //      delete shots[i];
  //    }
  //    delete[] shots;
  //  }
    RELEASE(surface);
    RELEASE(device);
    RELEASE(d3d);
    return hr;
}

DWORD WINAPI updateDegree(LPVOID lpParam)
{
	while (true)
	{
		if (playing)
		{
			if (CW)
				deg -= SHIP_V/100;
			else
				deg += SHIP_V/100;
				Sleep(10);
		}
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// spaceship O = (530, 140) - 64w x 70h
	// spaceship velocity : 31.5° in 0.298	105.7
	//						28.4° in 0.297	95.62
	//						30.2° in 0.300	100.66
	//						33.4° in 0.326	102.45
	//						~~~~~~~~~~~~~~~~~~~~~~	average = 101.1°/sec
	// r = 342 & d = 290
	// o = (534, 483)
	// ball velocity LEVEL 1 : 427.81 pixel/sec 
	// ball : 35x35, # of rgba 77,77,77,255 > than ~ 439

	LPDWORD dwThreadID = 0;
	LPDWORD dwThreadIDscrn = 0;
	HANDLE hThread = CreateThread(NULL, 0, updateDegree, NULL, 0, dwThreadID);
	HANDLE hThreadscrn = CreateThread(NULL, 0, Screenshot, NULL, 0, dwThreadIDscrn);

	BMP test;
	BMP AnImage;

	queue <ball> prospects;
	ball tmp;
	vector<ball> balls;

	SYSTEMTIME st;

	//for(int i=0;i<=100;++i) {printf("\r[%3d%%]",i);Sleep(100);}
	//printf("\n");

	//goto here;

	while(true)
    {
        if(GetAsyncKeyState(VK_NUMLOCK) && LASTNUMLOCK==0)
        {
            cout<<"Let's start the game - LEVEL" << LEVEL << "!\n";

			SetCursorPos(1300,489);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			
			playing = true;
			LASTNUMLOCK = 100;
        }

		if(GetAsyncKeyState(0x52) && LASTR==0)
		{
			playing = false;
            deg = 0;
			BALL_V *= 1.10;
			TAP_SLEEP *= 0.90;
			++LEVEL;
			LASTR = 100;
		}

		if (LASTNUMLOCK)
		{
			LASTNUMLOCK += 10;
			if (LASTNUMLOCK >=1010)
				LASTNUMLOCK = 0;
		}

		if (LASTR)
		{
			LASTR += 10;
			if (LASTR >=1010)
				LASTR = 0;
		}

		if (LASTNUMLOCK || LASTR)	Sleep(10);

    }


here:
	
	test.ReadFromFile("o-live9.bmp");

	GetSystemTime(&st);
	wprintf(L"copying BG... %i:%i:%i.%i", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
	AnImage.SetSize(1600,900);
	AnImage.SetBitDepth(32);

	for (int x=0;x<1600;++x)
	{
		for (int y=0;y<900;++y)
		{
			if ((int)test(x,y)->Red==77 && (int)test(x,y)->Green==77 && (int)test(x,y)->Blue==77)
				SCREEN[x][y] = true;

			AnImage(x,y)->Red = (int)test(x,y)->Red;
			AnImage(x,y)->Green = (int)test(x,y)->Green;
			AnImage(x,y)->Blue = (int)test(x,y)->Blue;
			AnImage(x,y)->Alpha = (int)test(x,y)->Alpha;			
		}
	}
	
			/*
		
			double curdeltaX = R * sin ((30)*PI/180);
			double curdeltaY = R - (R * cos ((30)*PI/180));

			//double deltaX = R * sin ((degreeCCW+deg)*PI/180);
			//double deltaY = R - (R * cos ((degreeCCW+deg)*PI/180));
			double curX = SHIP_OX - curdeltaX; 
			double curY = SHIP_OY + curdeltaY;	


			double shipxv = (curX - OX)/343;
			double shipyv = (curY - OY)/343;
			int Ishipxv, Ishipyv;
			for (int j=1;j<=343;++j)
			{
				Ishipxv = shipxv * j + OX;
				Ishipyv = shipyv * j + OY;
				cout << "(" << Ishipxv << "," << Ishipyv << ") <- line\n";
				AnImage(Ishipxv,Ishipyv)->Red = 0;
				AnImage(Ishipxv,Ishipyv)->Green = 100;
				AnImage(Ishipxv,Ishipyv)->Blue = 100;
				AnImage(Ishipxv,Ishipyv)->Alpha = 255;
			}

			cout << "V-> x:" << shipxv << " y:" << shipyv << "\n";
			*/

	GetSystemTime(&st);
	wprintf(L" - %i:%i:%i.%i DONE\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);	
	
	int sumFirstColumn,
		sumFirstRow,
		sumLastPlusOneColumn,
		sumLastPlusOneRow;

	int y_at = 0;
	
	GetSystemTime(&st);
	wprintf(L"\nindexing... %i:%i:%i.%i\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	//first 35x35
	//top left most point is (186,135)
	for (int x=186;x<221;++x)
		for (int y=135;y<170;++y)
			if (SCREEN[x][y])
				++map[0][0];
	if (map[0][0]>BALLPIXELS)
	{
		tmp.x = 0;
		tmp.y = 0;
		prospects.push(tmp);
	}

completeTheRow:
	//complete row
	for (int x=1;x<660;++x)
	{
		if (x>255 && x<406)
			if (y_at>255 && y_at<406) 
				continue;

		sumFirstColumn = 0;
		sumLastPlusOneColumn = 0;
		for (int y=135+y_at;y<170+y_at;++y)
		{
			if (SCREEN[x+185][y])
				++sumFirstColumn;
			if (SCREEN[x+220][y])
				++sumLastPlusOneColumn;			
		}
		map[x][y_at] = map[x-1][y_at] + sumLastPlusOneColumn - sumFirstColumn;
		if (map[x][y_at]>BALLPIXELS)
		{
			tmp.x = x;
			tmp.y = y_at;
			prospects.push(tmp);
		}
	}

	++y_at;

	if (y_at==659) { cout << "we have " << prospects.size() << " prospects! \n"; goto detect_balls; }

	for (int x=186;x<221;++x)
	{
		sumFirstRow = 0;
		sumLastPlusOneRow = 0;
		if (SCREEN[x][y_at+134])
			++sumFirstRow;
		if (SCREEN[x][y_at+169])
			++sumLastPlusOneRow;			
	}

	map[0][y_at] = map[0][y_at-1] + sumLastPlusOneRow - sumFirstRow;
	
	if (map[0][y_at]>BALLPIXELS)
	{
		tmp.x = 0;
		tmp.y = y_at;
		prospects.push(tmp);
	}

	if (y_at<660) goto completeTheRow;
	
	
detect_balls:
	
	while (!prospects.empty())	
	{
		int x = prospects.front().x;
		int y = prospects.front().y;

calibration:

		if (map[x+1][y]>map[x][y]) { ++x; goto calibration; }
		if (map[x-1][y]>map[x][y]) { --x; goto calibration; }
		if (map[x][y+1]>map[x][y]) { ++y; goto calibration; }
		if (map[x][y-1]>map[x][y]) { --y; goto calibration; }


		for (size_t j=0;j<balls.size();++j)
		{
			if (abs(x+186-balls[j].x)<=35 && abs(y+135-balls[j].y)<=35)
			{
					prospects.pop(); 
					goto detect_balls; 
			}
		}
		
		tmp.x = x+186;
		tmp.y = y+135;
				
		double distanceFromO = sqrt(pow(double(tmp.x+17.5-OX),2)+pow(double(tmp.y+17.5-OY),2))*D/R;
		double timeLeft = (D-distanceFromO)/BALL_V;
		double fullTime = D/BALL_V;
		double spentTime = fullTime - timeLeft;
		double xVelocity = (tmp.x+17.5-OX)/spentTime;
		double yVelocity = (tmp.y+17.5-OY)/spentTime;
		int finalX = xVelocity*timeLeft + tmp.x;
		int finalY = yVelocity*timeLeft + tmp.y;
		
		// SPACESHIP will be @
		// if going CCW
			
		double degreeCCW = SHIP_V * timeLeft;
		double current_degreeCCW = 47.2;
		double deltaX = R * sin ((degreeCCW+current_degreeCCW)*PI/180);
		double deltaY = R - (R * cos ((degreeCCW+current_degreeCCW)*PI/180));
			
		int nextX = SHIP_OX-deltaX; int nextY = SHIP_OY + deltaY;
		

		cout << "new ball @ (" << tmp.x << "," << tmp.y << ")\n";
		cout << "|__ it'll hit @ (" << finalX << "," << finalY << ") within " << timeLeft << "seconds\n";
		cout << "|__ we'll be  @ (" << nextX << "," << nextY << ")\n";
		cout << "hit?=" << hit(finalX,finalY,nextX,nextY) << "\n";
		
		if (hit(finalX,finalY,nextX,nextY))
		{
			SetCursorPos(1300,489);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			CW = !CW;
		}

		balls.push_back(tmp);
				

		for (int j=0;j<35;++j)
		{
			AnImage(tmp.x,tmp.y+j)->Red = 255;
			AnImage(tmp.x,tmp.y+j)->Green = 0;
			AnImage(tmp.x,tmp.y+j)->Blue = 0;
			AnImage(tmp.x,tmp.y+j)->Alpha = 255;
									   
			AnImage(tmp.x+35,tmp.y+j)->Red = 255;
			AnImage(tmp.x+35,tmp.y+j)->Green = 0;
			AnImage(tmp.x+35,tmp.y+j)->Blue = 0;
			AnImage(tmp.x+35,tmp.y+j)->Alpha = 255;
		}

		for (int j=0;j<35;++j)
		{
			AnImage(tmp.x+j,tmp.y)->Red = 255;
			AnImage(tmp.x+j,tmp.y)->Green = 0;
			AnImage(tmp.x+j,tmp.y)->Blue = 0;
			AnImage(tmp.x+j,tmp.y)->Alpha = 255;

			AnImage(tmp.x+j,tmp.y+35)->Red = 255;
			AnImage(tmp.x+j,tmp.y+35)->Green = 0;
			AnImage(tmp.x+j,tmp.y+35)->Blue = 0;
			AnImage(tmp.x+j,tmp.y+35)->Alpha = 255;
		}
	}
	GetSystemTime(&st);
	wprintf(L"found all balls... %i:%i:%i.%i\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);	

	std::cout << "finito!\n";
	//AnImage.WriteToFile("o-live-9-2.bmp");
	
	int t;
	//std::cin >> t;
	return 0;
}

