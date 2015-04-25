#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#pragma comment(lib, "d3dx11.lib")
#pragma warning( disable : 4996 )
#include <strsafe.h>
#pragma warning( default : 4996 )
#pragma comment(lib, "winmm.lib")
#include <time.h>
#include <list>
#include <algorithm>
#include <iostream>


using namespace std;

//error handling values
const int GERROR = 0;
const int ERROR32 = 1;
const int DXERROR = 2;

//Error Handling Function
int error(char* msg, int type = 0)
{
   char* title;
   switch(type)
   {
      case 0:
         title = "Error";
      break;   
      case 1:
         title = "Win32 Error";   
      break;
      case 2:
         title = "DirectX Error";
      break;
   }
   MessageBox(NULL, msg, title, MB_ICONEXCLAMATION | MB_OK);
   return 0;
}



//menu message number var
const int menuExit = 9001;
void addMenus(HWND handle)
{
   HMENU menu, fileMenu;

   menu = CreateMenu();

   fileMenu = CreatePopupMenu();

   AppendMenu(fileMenu, MF_STRING, menuExit, "E&xit");
   AppendMenu(menu, MF_STRING | MF_POPUP, (UINT)fileMenu, "&File");

   SetMenu(handle, menu);

}

//DxVariables
void renderFrame();
#define D3DFVF_Vertex (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

WPARAM handleMessages();

IDirect3D9 *g_D3D = NULL;
IDirect3DDevice9 *dxDevice = NULL;
IDirect3DVertexBuffer9 *dxVertexBuffer = NULL;
D3DPRESENT_PARAMETERS dxParams;
D3DVIEWPORT9 viewPort;


D3DXVECTOR3 eyeVector, lookAtVector,upVector;
D3DXMATRIX viewMatrix;
D3DXMATRIX projectionMatrix;
D3DXMATRIX worldMatrix;

float aspectRatio;
//vertex for directX
struct vertex
{
   float x, y, z, rhw;
   DWORD color;
};

//particles
//struct for holding the particle
struct particle
{
   vertex coords;
   int life;
   float r,g,b;
   bool alive;
};
//particle variables/settings
float fade = 0.5;
int pLife = 400;
const int updateRate = 50;
const int maxParticles = 10000;
int pPerFrame;
float gravity = 1, wind = 20;
int windChance = 10;
int pSpread = 10;
int renderType = 2;
byte pMaxRed = 150, pMaxGreen = 200, pMaxBlue = 250;
byte pMinRed = 0, pMinGreen = 50, pMinBlue = 100;

//simple x,y struct for points; require for "HotNoob" Text
struct point
{
   int x,y;
};

//Array for HotNoob
point HNPoints[] = {
   /*|*/{100,100},{100, 130},{100, 160},{100, 190},{100, 220},{100, 250},{100, 280},{100, 310},{100, 340},{100, 370},
   /*-*/{110,220},{120,220},{130,220},{140,220},{150,220},{160,220},{170,220},{180,220},{190,220},
   /*|*/{200,100},{200, 130},{200, 160},{200, 190},{200, 220},{200, 250},{200, 280},{200, 310},{200, 340},{200, 370},
   /*L*/{ 230, 220},{240,240}, {240, 200},{250,260},{250,180},
   /*M*/{ 260, 280},{260,160},{ 270, 280},{270,160},
   /*R*/{ 300, 220},{290,240}, {290, 200},{280,260},{280,180},
   /*-*/{ 330, 220},{ 340, 220},{ 350, 220},{ 360, 220},{ 370, 220},{ 380, 220},{ 390, 220},{ 400, 220},{ 410, 220},{ 420, 220},
   /*|*/{375,100},{375, 130},{375, 160},{375, 190},{375, 220},{375, 250},{375, 280},{375, 310},{375, 340},{375, 370},
   /*|*/{440,100},{440, 130},{440, 160},{440, 190},{440, 220},{440, 250},{440, 280},{440, 310},{440, 340},{440, 370},
   /*/*/{450,100},{460, 130},{470, 160},{480, 190},{490, 220},{500, 250},{510, 280},{520, 310},{530, 340},{540, 370},
   /*|*/{550,100},{550, 130},{550, 160},{550, 190},{550, 220},{550, 250},{550, 280},{550, 310},{550, 340},{550, 370},
   /*L*/{ 570, 220},{580,240}, {580, 200},{590,260},{590,180},
   /*M*/{ 600, 280},{600,160},{ 610, 280},{610,160},
   /*R*/{ 640, 220},{630,240}, {630, 200},{620,260},{620,180},
   /*L*/{ 660, 220},{670,240}, {670, 200},{680,260},{680,180},
   /*M*/{ 690, 280},{690,160},{ 700, 280},{700,160},
   /*R*/{ 730, 220},{720,240}, {720, 200},{710,260},{710,180},
   /*|*/{750,100},{750, 130},{750, 160},{750, 190},{750, 220},{750, 250},{750, 280},{750, 310},{750, 340},{750, 370},
    /*R*/{ 790, 320},{790,340}, {780, 300},{770,360},{770,280},{760,380},{760,260},
   /*_*/{100,450},{120,450},{140,450},{160,450},{180,450},{200,450},{220,450},{240,450},{260,450},{280,450},{300,450},{320,450},{340,450},{360,450},{380,450},{400,450},{420,450},{440,450},{460,450},{480,450},{500,450},{520,450},{540,450},{560,450},{580,450},{600,450},{620,450},{640,450},{660,450},{680,450},{700,450},{720,450},{740,450},{760,450},{780,450},{800,450}
   };
//for handling array
int HNPLength = sizeof(HNPoints)/sizeof(point);
int HNCount = 0;
//mouse location, 4 particle effect 1
POINTS mouse;
DWORD sTime = 0;
particle particles[maxParticles];
vertex data[maxParticles];
int pCount;
int maxRenderType = 3;

//initalization required for particles
void initalize()
{
   int i;
   for(i = 0; i < HNPLength; i++)
   {
      HNPoints[i].x -= 30;
   }
}
//update render settings; 4 renderTypes
void updateSettings()
{
   if(renderType == 0)
   {
      fade = 0.5;
      pLife = 400;
      pPerFrame;
      gravity = 1, wind = 20;
      windChance = 10;
      pSpread = 10;
      pMaxRed = 150, pMaxGreen = 200, pMaxBlue = 250;
      pMinRed = 0, pMinGreen = 50, pMinBlue = 100;
   }
   else if(renderType == 1)
   {
      pSpread = 2;
      windChance = 50;
      wind = 20.0f;
      fade = 2;
      pLife = 50;
      gravity = 0.7;
      pMaxRed = 100, pMaxGreen = 250, pMaxBlue = 250;
      pMinRed = 0, pMinGreen = 50, pMinBlue = 150;
   }
   else if(renderType == 2)
   {
      pSpread = 5;
      windChance = 50;
      wind = 20.0f;
      fade = 2;
      pLife = 200;
      gravity = 0.7;
      int i;
      pMaxRed = 250, pMaxGreen = 100, pMaxBlue = 250;
      pMinRed = 100, pMinGreen = 0, pMinBlue = 50;
   }
   else if(renderType == 3)
   {
      pSpread = 5;
      windChance = 50;
      wind = 20.0f;
      fade = 2;
      pLife = 200;
      gravity = 0.7;
      int i;
      pMaxRed = 250, pMaxGreen = 75, pMaxBlue = 250;
      pMinRed = 100, pMinGreen = 0, pMinBlue = 100;
   }
   pPerFrame = maxParticles/pLife + 3;
}
void changeRenderType()
{
   renderType++;
   if(renderType == maxRenderType+1)
   {
      renderType = 0;
   }
   updateSettings();
}


//misc variables
HRESULT hr;
bool dxOn = false;

//window variables
const int winWidth = 800, winHeight = 600;

//end func; clear all variables
void dispose()
{
   if(dxDevice)
   {
      dxDevice->Release();
      dxDevice = NULL;
   }
   if(g_D3D)
   {
      g_D3D->Release();
      g_D3D = NULL;
   }
   if(dxVertexBuffer)
   {
      dxVertexBuffer->Release();
      dxVertexBuffer = NULL;
   }
}

LRESULT CALLBACK WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch(msg)
   {
      case WM_PAINT:
         renderFrame();
         ValidateRect(handle, NULL);
      break;
      case WM_MOUSEMOVE:
         mouse = MAKEPOINTS(lParam);
         break;
      case WM_KEYUP:
         changeRenderType();
         break;
      case WM_CLOSE:
         DestroyWindow(handle);
      break;
      case WM_DESTROY:
         PostQuitMessage(0);
         dispose();
         break;
      case WM_COMMAND:
            switch(wParam)
            {
               case menuExit:
                  PostMessage(handle, WM_DESTROY, 0,0);
               break;
            }
         break;
      case WM_CREATE:
         addMenus(handle);
         break;
      default:
            return DefWindowProc(handle, msg, wParam, lParam); 
         break;
   }
   return 0;
}



const char* className = "WinClass";
void initalizeParticles()
{
   srand ( time(NULL) );
   pMaxRed -= pMinRed;
   pMaxGreen -= pMinGreen;
   pMaxBlue -= pMinBlue;
}
//Starting Point
int WINAPI WinMain(HINSTANCE handle, HINSTANCE cHandle, char* args, int windowState)
{
   //declare rand.
   WNDCLASSEX win;
   win.cbSize = sizeof(WNDCLASSEX);
   win.lpszClassName = className;
   win.lpszMenuName = NULL;
   win.style = 0;
   win.cbClsExtra = 0;
   win.cbWndExtra = 0;
   win.hIcon = LoadIcon(NULL, IDI_APPLICATION);
   win.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
   win.hCursor = LoadCursor(NULL, IDI_APPLICATION);
   win.hbrBackground = CreateSolidBrush(RGB(50, 50, 50));
   win.hInstance = handle;
   win.lpfnWndProc = WndProc;

   if(!RegisterClassEx(&win))
   {
      return error("Failed To Register Class", ERROR32);
   }

   HWND fHandle;
   fHandle = CreateWindowEx(
                       WS_EX_CLIENTEDGE,
                       className,
                       "DirectX Progarm",
                       WS_OVERLAPPED,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       winWidth,
                       winHeight,
                       NULL,
                       NULL,
                       handle,
                       NULL);
   if(fHandle == NULL)
   {
      return error("Failed To Create Window-Ex", ERROR32);
   }
   
   ShowWindow(fHandle, windowState);
   UpdateWindow(fHandle);

   g_D3D = Direct3DCreate9(D3D_SDK_VERSION);
   if(!g_D3D)
   {
      return error("Unable To Create DirectX 3D Object", DXERROR);
   }

   ZeroMemory(&dxParams,sizeof(dxParams));

   dxParams.BackBufferCount = 1;
   dxParams.MultiSampleType = D3DMULTISAMPLE_NONE;
   dxParams.MultiSampleQuality = 0;
   dxParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
   dxParams.hDeviceWindow = fHandle;
   dxParams.Flags = 0;
   dxParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
   dxParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
   dxParams.BackBufferFormat = D3DFMT_R5G6B5;
   dxParams.EnableAutoDepthStencil = false;

   
   dxParams.Windowed = true;
   
   hr = g_D3D->CreateDevice(D3DADAPTER_DEFAULT,
                  D3DDEVTYPE_HAL,
                  fHandle,
                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                  &dxParams,
                  &dxDevice);

   if(FAILED(hr))
   {
      return error("Failed To Create DirectX Device", DXERROR);
   }

   initalizeParticles();

   //Create Buffer
   int vertexSize = sizeof(particles);
   int vertexCount = vertexSize/sizeof(vertex);
   
   hr = dxDevice->CreateVertexBuffer(vertexSize,
                             0, 
                             D3DFVF_Vertex,
                             D3DPOOL_DEFAULT,
                             &dxVertexBuffer,
                             NULL);
   if(FAILED(hr))
   {
      return error("Failed To Create Vertex Buffer.", DXERROR);
   }


    dxDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting, since we are providing our own vertex colors
    dxDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

   
   initalize();
   updateSettings();
   dxOn = true;
   return handleMessages();
}
void addParticles(int newParticles)
{
   int mx, my;
   if(renderType == 0)
   {
      if(mouse.x > winWidth)
      {
         mx = winWidth;
      }
      else if(mouse.x < 0)
      {
         mx = 0;
      }
      else
      {
         mx = mouse.x;
      }
      if(mouse.y > winHeight)
      {
         my = winWidth;
      }
      else if(mouse.y < 0)
      {
         my = 0;
      }
      else
      {
         my = mouse.y;
      }
   }
   if(renderType == 2)
      {
         mx = HNPoints[HNCount].x;
         my = HNPoints[HNCount].y;
         HNCount++;
         if(HNCount > HNPLength)
         {
            HNCount = 0;
         }
      }
   else if(renderType == 3)
   {
      mx = HNPoints[HNCount].x;
      my = HNPoints[HNCount].y;
      HNCount--;
      if(HNCount == -1)
      {
         HNCount = HNPLength;
      }
   }
   int i, nPCount = 0;
   for(i = 0; i < maxParticles; i++)
   {
      if(renderType == 1)
      {
         int index = rand() % HNPLength;
         mx = HNPoints[index].x;
         my = HNPoints[index].y;
      }
      if(particles[i].alive == false)
      { 
         particles[i];
         particles[i].alive = true;
         particles[i].coords.x = mx + (rand() % pSpread*2)-pSpread;
         particles[i].coords.y = my + (rand() % pSpread*2)-pSpread;
         particles[i].coords.z = 1;
         particles[i].coords.rhw = 1;
         particles[i].coords.color = RGB(particles[i].r, particles[i].g, particles[i].b);
         particles[i].life = pLife;
         particles[i].r = (rand() % pMaxRed) + pMinRed;
         particles[i].g = (rand() % pMaxGreen) + pMinGreen;
         particles[i].b = (rand() % pMaxBlue) + pMinBlue;
         nPCount++;
         if(nPCount == newParticles)
         {
            return;
         }
      }
   }
}
byte minZero(byte number)
{
   if(number < 0)
   {
      number = 0;
   }
   return number;
}
void updateParticles()
{
   int i, dCount = 0;
   for(i = 0; i < maxParticles; i++)
   {
      if(particles[i].alive)
      {
         particles[i].coords.color = particles[i].coords.color = D3DCOLOR_XRGB((int)particles[i].r, (int)particles[i].g, (int)particles[i].b);
         data[dCount] = particles[i].coords;
         dCount++;
         particles[i].coords.y += gravity;
         if(rand() % windChance == 1)
         {
            particles[i].coords.x += (float)(rand() % (int)wind);
         }
         if(particles[i].r > fade)
         {
            particles[i].r -= fade;
         }
         if(particles[i].g > fade)
         {
            particles[i].g -= fade;
         }
         if(particles[i].b > fade)
         {
            particles[i].b -= fade;
         }
         particles[i].life -= 1;

         if(particles[i].life == 0)
         {
            particles[i].alive = false;
         }
         else if(particles[i].r <= fade &&
               particles[i].g <= fade &&
               particles[i].b <= fade)
         {
            particles[i].alive == false;
         }
      }
   }
   pCount = dCount;
}
void updateBuffer()
{
   void* Verticles;
   hr = dxVertexBuffer->Lock(0,0, &Verticles,0);
   if(FAILED(hr))
   {
      error("Failed To Lock Vertex Buffer.", DXERROR);
      return;
   }
      memcpy(Verticles, &data, sizeof(vertex) * pCount);
   hr = dxVertexBuffer->Unlock();
   if(FAILED(hr))
   {
      error("Failed To UnLock Vertex Buffer.", DXERROR);
      return;
   }
}
void renderParticles()
{
    DWORD cTime = GetTickCount();
   if(sTime - cTime > updateRate)
   {
      sTime = cTime;
      updateParticles();
      addParticles(pPerFrame); 
      updateParticles();
      updateBuffer();
   }
}
void renderFrame()
{
   if(!dxOn)
   {
      return;
   }
   //MessageBox(NULL, "rendering", "rendering", MB_OK);
   hr = dxDevice->TestCooperativeLevel();
   if(hr == D3DERR_DEVICELOST)
   {
      Sleep(500);
      error("Device Lost", DXERROR);
      return;
   }
   else if(hr == D3DERR_DEVICENOTRESET)
   {
      hr = dxDevice->Reset(&dxParams);
      if(SUCCEEDED(hr))
      {

      }
   }

   //render before clear; for no flash
   renderParticles();

   hr = dxDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);
   if(FAILED(hr))
   {
      error("Failed To Clear DxDevice", DXERROR);
      return;
   }


   hr = dxDevice->BeginScene();
   if(FAILED(hr))
   {
      error("Failed To Begin Scene.", DXERROR);
      return;
   }

   hr = dxDevice->SetFVF(D3DFVF_Vertex);
   if(FAILED(hr))
   {
      error("Failed To SetFVF", DXERROR);
      return;
   }

   dxDevice->SetStreamSource(0, dxVertexBuffer,0,sizeof(vertex));
   dxDevice->DrawPrimitive(D3DPT_POINTLIST,0, pCount);
   
   //dxDevice->DrawPrimitiveUP(D3DPT_POINTLIST, pCount, &data, sizeof(vertex)); 



   //end render

   hr = dxDevice->EndScene();
   if(FAILED(hr))
   {
      error("Failed To End Scene.", DXERROR);
      return;
   }
   hr = dxDevice->Present(NULL, NULL, NULL, NULL);
   if(FAILED(hr))
   {
      error("Failed To Present Scene");
      return;
   }
}
WPARAM handleMessages()
{
   MSG msg;
   ZeroMemory( &msg, sizeof( msg ) );
   while( msg.message != WM_QUIT )
   {
      if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
      {
         TranslateMessage( &msg );
         DispatchMessage( &msg );
      }
      else
      {
         renderFrame();
      }
   }
   return msg.wParam;
}