﻿// magnifier.cpp : 定义应用程序的入口点。
//
#include <shellscalingapi.h>
#include <WtsApi32.h>
#include "framework.h"
#include "magnifier.h"
#include "desktop_capture/desktop_cpature_include.h"
#include "FakeSharedMemoryFactory.h"
#include "GDIRender.h"

#include <iostream>

#define MAX_LOADSTRING 100

using namespace webrtc;
static std::unique_ptr<DesktopCapturer> _webrtcCap;
std::unique_ptr<SharedMemoryFactory> _sharedMemoryFactory;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hWnd;

typedef  BOOL (WINAPI *funWTSRegisterSessionNotification)(HWND hWnd, DWORD dwFlags);
typedef  BOOL (WINAPI *funWTSUnRegisterSessionNotification)(HWND hWnd);
static funWTSRegisterSessionNotification pWTSRegisterSessionNotification = nullptr;
static funWTSUnRegisterSessionNotification pWTSUnRegisterSessionNotification = nullptr;


class MagCallback : public DesktopCapturer::Callback {
public:
    MagCallback() {
        _render.reset(new CGDIRender());
    };
    // Called after a frame has been captured. |frame| is not nullptr if and
    // only if |result| is SUCCESS.
    virtual void OnCaptureResult(DesktopCapturer::Result result,
        std::unique_ptr<DesktopFrame> frame)
    {
        _render->render(hWnd, frame);
    };

    virtual ~MagCallback() {
    }
private:
    std::unique_ptr<CGDIRender> _render;
};

static std::unique_ptr<MagCallback> _webrtcCallback;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#pragma comment(lib, "Shcore.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MAGNIFIER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAGNIFIER));

    MSG msg;

    SetTimer(hWnd, 0, 100, NULL);

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if(pWTSUnRegisterSessionNotification)
        pWTSUnRegisterSessionNotification(hWnd);

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAGNIFIER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MAGNIFIER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 1280, 720, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   if (!pWTSRegisterSessionNotification) {
       HMODULE hModule = LoadLibraryW(L"Wtsapi32.dll");
       if (hModule) {
           pWTSRegisterSessionNotification = reinterpret_cast<funWTSRegisterSessionNotification>(GetProcAddress(hModule, "WTSRegisterSessionNotification"));
           pWTSUnRegisterSessionNotification = reinterpret_cast<funWTSUnRegisterSessionNotification>(GetProcAddress(hModule, "WTSUnRegisterSessionNotification"));
       }
   }
   
   if(pWTSRegisterSessionNotification)
        pWTSRegisterSessionNotification(hWnd, NOTIFY_FOR_THIS_SESSION);
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_START:
                {
                    DesktopCaptureOptions options;
                    _webrtcCap = DesktopCapturer::CreateScreenCapturer(options);
                    _webrtcCallback.reset(new MagCallback());
                    //_sharedMemoryFactory.reset(new FakeSharedMemoryFactory());
                    _webrtcCap->SetSharedMemoryFactory(std::unique_ptr<SharedMemoryFactory>(new FakeSharedMemoryFactory()));
                    _webrtcCap->SelectSource(0);
                    _webrtcCap->Start(_webrtcCallback.get());
                }
                break;
            case IDM_STOP:
                {
                    _webrtcCap.reset(nullptr);
                    //DesktopCaptureOptions options;
                    //_webrtcCap = DesktopCapturer::CreateScreenCapturer(options);
                    //_webrtcCallback.reset(new MagCallback());
                    //_webrtcCap->Start(_webrtcCallback.get());
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
        {
            if(_webrtcCap)
                _webrtcCap->CaptureFrame();
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_DPICHANGED:
    case WM_DISPLAYCHANGE:
        {
            _webrtcCap.reset(nullptr);
            DesktopCaptureOptions options;
            _webrtcCap = DesktopCapturer::CreateScreenCapturer(options);
            //_sharedMemoryFactory.reset(new FakeSharedMemoryFactory());
            _webrtcCap->SetSharedMemoryFactory(std::unique_ptr<SharedMemoryFactory>(new FakeSharedMemoryFactory()));
            _webrtcCap->SelectSource(0);
            _webrtcCap->Start(_webrtcCallback.get());
        }
        break;
    case WM_WTSSESSION_CHANGE:
    {
        switch (wParam) {
        case WTS_SESSION_LOCK:
            MessageBox(NULL, L"WTS_SESSION_LOCK", L"ERROR", MB_OK);
            break;
        case WTS_SESSION_UNLOCK:
            MessageBox(NULL, L"WTS_SESSION_UNLOCK", L"ERROR", MB_OK);
            break;
        }
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
