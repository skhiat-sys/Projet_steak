#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "stega.cpp"

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#define ID_FILE_OPEN_IMAGE 4001
#define ID_FILE_QUIT       4002

Bitmap* g_pBitmap = nullptr;
Steganographie sten;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static std::string texte(500, 0);
static HWND hEdit;
static std::vector<unsigned char> bmpD;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    ULONG_PTR gdiToken;
    GdiplusStartupInput gdiInput;
    GdiplusStartup(&gdiToken, &gdiInput, NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ImageViewer";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);


    HWND hwnd = CreateWindowEx(
        0,
        L"ImageViewer",
        L"Visionneuse d'Images",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        900, 700,
        NULL, NULL,
        hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiToken);
    return 0;
}

bool DoOpenImage(HWND hwnd)
{
    wchar_t path[MAX_PATH] = L"";

    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Images\0*.png;*.jpg;*.jpeg;*.bmp\0Tous\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;

    if (!GetOpenFileName(&ofn))
        return false;

    delete g_pBitmap;
    g_pBitmap = new Bitmap(path);

    std::wstring wsPath = path;
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wsPath.c_str(),
        (int)wsPath.size(),
        NULL, 0, NULL, NULL);

    std::string strPath(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wsPath.c_str(),
        (int)wsPath.size(),
        &strPath[0], size_needed, NULL, NULL);

    bmpD = sten.ReadBMP(strPath);

    if (g_pBitmap->GetLastStatus() != Ok)
    {
        delete g_pBitmap;
        g_pBitmap = nullptr;
        MessageBox(hwnd, L"Impossible de charger l'image.", L"Erreur", MB_ICONERROR);
        return false;
    }

    InvalidateRect(hwnd, NULL, TRUE);
    return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {

        hEdit = CreateWindowEx(
            0, L"Edit", L"Ouvrir une image",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |     // Barre de défilement verticale
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            20, 600, 1500, 300,
            hwnd, (HMENU)3, NULL, NULL
        );


        HMENU hMenu = CreateMenu();
        HMENU hFile = CreatePopupMenu();
        HMENU hFile2 = CreatePopupMenu();

        AppendMenu(hFile, MF_STRING, ID_FILE_OPEN_IMAGE, L"Ouvrir une image");
        AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
        AppendMenu(hFile, MF_STRING, ID_FILE_QUIT, L"Quitter");

        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, L"Fichier");


        AppendMenu(hFile2, MF_STRING, 1, L"chiffrer");
        AppendMenu(hFile2, MF_STRING, 2, L"dechiffre");

        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile2, L"steganographie");
        SetMenu(hwnd, hMenu);
    }
    return 0;

    case WM_SIZE:
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_OPEN_IMAGE:
            DoOpenImage(hwnd);
            break;
        case 3:
            WCHAR buffer[256];
            GetWindowTextW(hEdit, buffer, 256);

            static int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
            

            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &texte[0], size, NULL, NULL);
            break;
        case ID_FILE_QUIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case 1:
            sten.EmbedLSB(bmpD, texte);
            sten.WriteBMP("output.bmp", bmpD);

            delete g_pBitmap;
            g_pBitmap = new Bitmap(L"output.bmp");

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case 2:
            std::string msg = sten.ExtractLSB(bmpD);

            std::wstring out(msg.begin(), msg.end());
            SetWindowTextW(hEdit, out.c_str());
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (g_pBitmap)
        {
            Graphics g(hdc);

            RECT rc;
            GetClientRect(hwnd, &rc);
            int winW = rc.right - rc.left;
            int winH = rc.bottom - rc.top;

            int imgW = g_pBitmap->GetWidth();
            int imgH = g_pBitmap->GetHeight();

            float scaleX = (float)winW / imgW;
            float scaleY = (float)winH / imgH;
            float scale = min(scaleX, scaleY);

            int finalW = (int)(imgW * scale);
            int finalH = (int)(imgH * scale);

            int posX = (winW - finalW) / 2;
            int posY = (winH - finalH) / 2;

            g.DrawImage(g_pBitmap, posX, posY, finalW, finalH);
        }

        EndPaint(hwnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        delete g_pBitmap;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
